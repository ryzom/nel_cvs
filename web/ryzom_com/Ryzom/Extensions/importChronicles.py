# -*- coding: utf-8 -*-
from Products.CMFCore.utils import getToolByName
import MySQLdb
import re

def createDraft(self, target, id, title, desc, publishDate, excerpt, text,topics,lang,author):
	target.invokeFactory(id=id, type_name='WeblogEntry', title=title,description=desc, text=text)
	new_obj = getattr(target, id)
	new_obj.setEffectiveDate(publishDate)
	new_obj.setModificationDate(publishDate)
	new_obj.setEntryCategories(topics)
	new_obj.setDescription(excerpt)
	new_obj.setLanguage(lang)

	if lang=='en':		
		try:
			new_obj.setText(text.replace('\xc2','').decode('cp1252').encode('utf'),mimetype='text/html')
		except:
			new_obj.setText(text,mimetype='text/html')
		new_obj.setCreators(author)
	else:
		try:
			new_obj.setText(text.replace('\xc2','').decode('cp1252').encode('utf'),mimetype='text/html')
		except:
			try:
				new_obj.setText(text.decode('utf').encode('latin'),mimetype='text/html')
			except:
				new_obj.setText(text.decode('latin'),mimetype='text/html')
		try:
			new_obj.setCreators(author)
		except:
			try:
				new_obj.setCreators(author.decode('utf').encode('latin'),mimetype='text/html')
			except:
				new_obj.setCreators(author.decode('latin'),mimetype='text/html')

	return new_obj


def addTopic(self, newblog, newId, newName, existingTopics):
	""" add a category to the blog - but only, if it doesn't exist yet."""
	if newId not in [blogTopic.getId() for blogTopic in existingTopics]:
		self.plone_log("Adding category [%s]" %newId)
		newblog.invokeFactory('WeblogTopic', id=newId, title=newName)


def mod_rewrite_rule(self, cb_entry, id, path,year,month,day):
	dest_url = "/" + path + year + "/" + month + "/" + day + "/" + id
	source_url = "^/blog/" + cb_entry + ".*"
	rule = "RewriteRule " + source_url + " " + dest_url
	return rule

def importChronicles(self):
	portal = getToolByName(self,'portal_url').getPortalObject()
	quillsBlog = 'chronicles'
	drafts_path = quillsBlog + '/drafts/'
	archive_path = quillsBlog + '/archive/'
	rewrite_rules = ""
	target = portal.restrictedTraverse(drafts_path)
	# Create the blog based on the values set above
	newblog = getattr(portal, quillsBlog)
	self.plone_log("Found new blog [%s]" %newblog.title)
	qtool = portal.quills_tool
	dtool = portal.portal_discussion
	blogTopics = newblog.getTopics()

	taghtml=re.compile('<.*?>')

	connectionObject = MySQLdb.connect(host='localhost', user='bernard', passwd='password', db='toto')
	c = connectionObject.cursor()
	c.execute("""SELECT distinct * FROM ryzom_news where rubrique='rp_features_fr' or rubrique='rp_features' or rubrique='rp_features_de' or rubrique='rp_news_fr' or rubrique='rp_news' or rubrique='rp_news_de' or rubrique='rp_opinion_fr' or rubrique='rp_opinion' or rubrique='rp_opinion_de' ORDER BY id;""")

	for row in c.fetchall():
		id=row[0]
		day=str(row[1])
		month=str(row[2])
		year=str(row[3])
		hour=str(row[4])
		min=str(row[5])
		title=row[6].replace('&#8217;','\'')
		image=row[7]
		body=row[8]
		band=row[9]
		blason=row[10]
		rubrique=row[11]
		stick=row[12]

		#toute les rubriques sont de la forme rp_***_language sauf l'anglais qui va provoquer une erreur 
		try:
			lang=rubrique.split('_')[2]
		except:
			lang='en'
		topics=rubrique.split('_')[1]

		body=body.replace('<p align="justify"> </p>','').replace('<em>   </em>','')
		body=body.replace('<div align="justify"> </div>','').replace('<p>   </p>','')
		body=body.replace('<p> </p>','').replace('<p class="MsoNormal"> </p>','')
		body=body.replace('<p class="MsoNormal"><br /></p>','').replace('<p><br />','')
		body=body.replace('<em />','').replace('<font face="Times New Roman" size="3" />','')
		body=body.replace('<strong />','')
		if id==1564:
			body=body.replace('<font size="3" face="Times New Roman">','').replace('</font>','').replace	('<font size="3">','').replace('<font face="Times New Roman">','').replace('<font face="Times New Roman" />','')
		if id==1484 or id==1604:
			body=body.replace('<font size="3" />','').replace('<font size="3">','').replace('<p align="left"></p>','')
			body=body.replace('<font face="Times New Roman" size="3">','').replace('</font>','')
			body=body.replace('<p align="left"></p>','').replace('<font face="Times New Roman">','')
		if id==1386:
			body=body.replace('<strong><strong><strong><strong><strong><strong><strong>','')
			body=body.replace('</strong></strong></strong></strong></strong></strong></strong>','')
		if id==1481:
			body=body.replace('<div align="center"><pre><span style="font-size: 12pt;"> </span></pre> </div>','')

		ok=False
		if len(body.split('[split]',1))==2 and ok==False:
			excerpt=body.split('[split]',1)[0]	
			text=body.split('[split]',1)[1]
			ok=True
		elif len(body.split('<p>&nbsp;</p>',1))==2 and ok==False:
			excerpt=body.split('<p>&nbsp;</p>',1)[0]
			text=body.split('<p>&nbsp;</p>',1)[1]
			ok=True
		elif len(body.split('<br />',1))==2 and ok==False:
			excerpt=body.split('<br />',1)[0]
			text=body.split('<br />',1)[1]
			ok=True
		elif len(body.split('</p>',1))==2 and ok==False:
			excerpt=body.split('</p>',1)[0]
			text=body.split('</p>',1)[1]
		if id==1519:
			excerpt=body.split('. ',1)[0]
			text=body.split('. ',1)[1]

		#fix the right path for chronicles images
		blason=re.sub('images/','/chronicles/images/',blason)
		#insert "blason" in the top-left of the page	
		text='<div class="weblog_chronicles"> <img class="weblog_blason" style=\"position: relative;\" src=\"'+blason+'\" align=\"right\" height=\"278\" width=\"170\">'+text+'</div>'
		
		#need to clean &...; to real character
		excerpt=taghtml.sub('',excerpt)
		try:
			excerpt=excerpt.replace('\xc2','').decode('cp1252').encode('utf')
		except:
			try:
				excerpt=excerpt.decode('latin')
			except:
				pass
		excerpt=excerpt.replace('&eacute;','é').replace('&egrave;','è').replace('&ecirc;','ê').replace('&acute;','´')
		excerpt=excerpt.replace('&agrave;','à').replace('&acirc;','â').replace('&ocirc;','ô').replace('&ucirc;','û')
		excerpt=excerpt.replace('&nbsp;',' ').replace('&quot;','\"').replace('&Agrave;','A').replace('&icirc;','î')
		excerpt=excerpt.replace('&uuml;','ü').replace('&ouml;','ö').replace('&auml;','ä').replace('&iuml;','ï')
		excerpt=excerpt.replace('&szlig;','ß').replace('&ccedil;','ç')
		excerpt=excerpt.replace('&raquo;','»').replace('&laquo;','«').replace('&ugrave;','ù')
		excerpt=excerpt.replace('&rsquo;','’').replace('&ndash;','–').replace('&hellip;','…')
		excerpt=excerpt.replace('&Uuml;','Ü').replace('&Iuml;','Ï').replace('&Ouml;','Ö')
		excerpt=excerpt.replace('&Auml;','Ä').replace('&oelig;','œ').replace('&lt;','<').replace('&lsquo;','‘')
		excerpt=excerpt.replace('&bdquo;','„').replace('&ldquo;','“').replace('&rdquo;','”')

		#creation du topics s'il n'existe pas
		try:
			addTopic(self, newblog,topics,topics, blogTopics)
		except:
			pass

		#on complete les rubriques
		date=year+"-"+month+"-"+day+" "+hour+":"+min+":00"
		desc=''
		
		if lang=='en':
			author='zerlin'
		elif lang=='fr':
			author='choule'
		elif lang=='de':
			author='virien'

		#gestion du codage latin, utf et windows-1252 (zarb)
		try:
			title=title.replace('\xc2','').decode('cp1252').encode('utf')
		except:
			try:
				title=title.decode('latin')
			except:
				pass

		new_id = qtool.idFromTitle(title, 'WeblogEntry', newblog)
		
		try:
			new_entry=createDraft(self,target,new_id,title,desc,date,excerpt,text,topics,lang,author)
		except:
			new_id=new_id+str(id)
			new_entry=createDraft(self,target,new_id,title,desc,date,excerpt,text,topics,lang,author)		
		rewrite_rules=rewrite_rules + "\r" + mod_rewrite_rule(self, title,new_id,archive_path,str(year),str(month),str(day))
	return rewrite_rules
