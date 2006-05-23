# -*- coding: utf-8 -*-
from Products.CMFCore.utils import getToolByName
import MySQLdb
import re


def rewriteLink(d,text,oldid):
	f = open ("error.txt", "a")
	liens=re.findall('http://www.ryzom.com/\?page=news\&id=\d*',text)
	for i in liens:
		#on recupere l'id
		idlink=i.split('id=')[1]
		#on remplace le liens par le nouveau chemin
		try:
			c=str(d[str(idlink)])
		except:
			f.write("probleme de dico:"+str(idlink)+"\n")
		try:			
			text=re.sub('http://www.ryzom.com/\?page=news\&id=\d*',c,text,count=1)
		except:
			f.write(str(oldid)+"_appel_"+str(idlink)+"\n")

	liensimages=re.findall('http://www.ryzom.com/images/\S*\....',text)	
	for i in liensimages:
		#on remplace le liens par le nouveau chemin
		l=i.split('/images')[1]
		try:			
			text=re.sub('http://www.ryzom.com/images/\S*\....','/ryzom/images'+l,text,count=1)
		except:
			f.write(str(oldid)+"_appelimages_"+l+"\n")

	liensdata=re.findall('http://www.ryzom.com/data/\S*\....',text)
	for i in liensdata:
		#on remplace le liens par le nouveau chemin
		l=i.split('/data')[1]
		try:			
			text=re.sub('http://www.ryzom.com/data/\S*\....','/ryzom/data'+l,text,count=1)
		except:
			f.write(str(oldid)+"_appeldata_"+l+"\n")
	f.close()
	return text

def createDraft(self, target, id, title, desc, publishDate, excerpt, text,topics,lang,author):	
	if lang=='en':
		target.invokeFactory(id=id, type_name='WeblogEntry', title=title,description=desc, text=text)
		new_obj = getattr(target, id)
		new_obj.setEffectiveDate(publishDate)
		new_obj.setModificationDate(publishDate)
		new_obj.setEntryCategories(topics)
		new_obj.setDescription(excerpt)
		try:
			new_obj.setText(text.replace('\xc2','').decode('cp1252').encode('utf'))
		except:
			new_obj.setText(text)
		new_obj.setCreators(author)
		new_obj.setLanguage(lang)
	else:
		target.invokeFactory(id=id, type_name='WeblogEntry', title=title,description=desc, text=text)
		new_obj = getattr(target, id)
		new_obj.setEffectiveDate(publishDate)
		new_obj.setModificationDate(publishDate)
		new_obj.setEntryCategories(topics)
		new_obj.setDescription(excerpt)		
		try:
			new_obj.setText(text.replace('\xc2','').decode('cp1252').encode('utf'))
		except:
			try:
				new_obj.setText(text.decode('utf').encode('latin'))
			except:
				new_obj.setText(text.decode('latin'))
		try:
			new_obj.setCreators(author)
		except:
			try:
				new_obj.setCreators(author.decode('utf').encode('latin'))
			except:
				new_obj.setCreators(author.decode('latin'))
		new_obj.setLanguage(lang)
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

def importMySQL2Quills(self):
	portal = getToolByName(self,'portal_url').getPortalObject()
	quillsBlog = 'news'
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

	link={'':''}
	connectionObject = MySQLdb.connect(host='localhost', user='bernard', passwd='password', db='toto')
	c = connectionObject.cursor()
	c.execute("""SELECT distinct * FROM ryzom_news where rubrique='ryzom.fr' or rubrique='ryzom.de' or rubrique='ryzom.com' ORDER BY id;""")
	for row in c.fetchall():
		id=row[0]
		day=str(row[1])
		month=str(row[2])
		year=str(row[3])
		hour=str(row[4])
		min=str(row[5])
		title=row[6]
		body=row[7]
		more=row[8]
		author=row[9]
		topics=row[10]
		language=row[11]
		type2=row[12]

		if language=='ryzom.fr':
			lang='fr'
		if language=='ryzom.com':
			lang='en'
		if language=='ryzom.de':
			lang='de'

		#auteur par default
		author=author.lower()
		if (author==''
			or author=='xavier antoviaque'
			or author=='xavier antoviaque [translation]'
			or author=='xavier antoviaque [traduction]'
			or author==' xavier antoviaque [traduit par choule]'
			or author=='xavier antoviaque [traduit par choule]'
			or author=='xavier antoviaque [traduit par choule et colorman]'
			or author=='xavier antoviaque [adaptÃ© par choule]'
			or author=='xavier antoviaque [traduit par colorman]'
			or author=='xavier antoviaque [traduit par les csrs]'
			or author=='xavier antoviaque [traduit par colorman et choule]'
			or author=='xavier antoviaque [traduit par canestel et choule]'
			or author=='xavier antoviaque [traduit par choule et ciryatan]'
			or author=='xavier antoviaque [traduit par choule et marjo]'
			or author=='xavier antoviaque [Ã¼bersetzt von kadael]'
			or 'xavier antoviaque' in author):
			author='antoviaque'
		if (author=='jessica mulligan'
			or author=='jessica mulligan [traduit par colorman]'
			or author=='jessica mulligan [traduction]'
			or author=='jessica mulligan [traduit par colorman et choule]'
			or author=='jessica mulligan [traduit par choule]'
			or author=='jessica mulligan [traduit par tanreal et choule]'
			or author=='jessica mulligan [traduit par canestel et choule]'
			or author=='jessica mulligan [traduit par ciryatan et choule]'):
			author='mulligan'
		if (author=='marjo'
			or author=='marjo [traduit par choule]'
                        or author=='marjo [traduit par chouleet enoril]'
			or author=='marjo [Ã¼bersetzt von kadael]'
			or author=='marjo [Ã¼bersetzt von chaeitch & kadael]'				    
                        or author=='marjo [Ã¼bersetzt von yumidan]'
			or author=='marjo (Ã¼bersetzt von yumidan)'
			or 'marjo' in author):
			author='Marjo'
		if (author=='david cohen corval'
			or author=='david cohen corval [traduit par choule]'
			or author=='david cohen corval [traduit par tanreal]'
			or author=='david cohen corval [traduit par canestel et choule]'
			or author=='david cohen corval [traduit par illumiel et choule]'):
			author='cohen'
		if (author=='lawrence' or author=='lawrence [traduit par choule]'):
			author='lawrence'
		if (author=='vince (head of customer support)'
			or author=='vince (lead Support clientÃ¨le)'
			or author=='vince (lead support clientÃ¨le) [traduit par choule]'
                        or author=='vince (leiter des kundenservices von nevrax) [Ã¼bersetzt von kadael]'
			or author=='vince, head of customer support'
			or 'vince' in author):
			author='vince'
		if (author=='soulsnatcher' or author=='soulsnatcher [traduit par choule]'):
			author='sousnatcher'
		if author=='virien':
			author='virien'
		if author=='kadael':
			author='kadael'
		if author=='colorman':
			author='colorman'
		if author=='adra, twanaar & sherca':
			author='adra'
		if author=='alexis aubenque':
			author='aubenque'
		if author=='nevrax':
			author='nevrax'
		if author=='shadowweb':
			author='shadowweb'


		topics=topics.lower()
		if (topics=='q&a'
			or (topics=='news' and lang=='fr')
			or topics=='fragen & antworten'
                        or topics=='q&r'):
			topics='qa'
		if (topics=='proposition' or topics=='planung'):
			topics='proposed'
		if topics=='episode 2':
			topics='episode'
		if topics=='forum watch':
			topics='forums'
		if (topics=='forum'
			or topics=='regard sur les forums'
			or topics=='foren'
			or topics=='dev watch'):
			topics='forums'
		if (topics=='tip' or topics=='astuce'):
			topics='tips'
		if (topics=='job' or topics=='postes'):
			topics='jobs'
		if (topics=='paper'
			or topics=='entwicklung'
			or topics=='dev'
			or topics=='hinweis'
			or topics=='umfrage'
			or topics=='news'
			or topics=='in development'
			or topics=='en d\xe9veloppement'
			or topics=='in entwicklung'
			or topics=='produktion'):
			topics='production'		

		if (topics=='technique'
			or topics=='technik'
			or topics=='technischer hinweis'
                        or topics=='technisch'
			or topics=='wartung'):
			topics='technical'
		if (topics=='l\xe9galit\xe9'
                        or topics=='regelwerk'
                        or topics=='spielregeln'):
			topics='policy'
		if (topics=='concours'
			or topics=='wettbewerb'
			or topics=='story-wettbewerb'
			or topics=='trivia'):
			topics='contest'
		if topics=='animation':
			topics='events'
		if (topics=='website'
			or topics=='site web'
			or topics=='siteweb'
			or topics=='site internet'
			or topics=='webseite'):
			topics='web'
		if topics=='brief':
			topics='letter'
		if (topics=='lettre'
			or topics=='infos'
			or topics=='in planung'):
			topics='letter'
		if (topics=='histoire'
                        or topics=='geschichte'):
			topics='story'
		if topics=='event' and lang=='de':
			topics='story'
		if topics=='presse':
			topics='press'		
		if topics=='chronologie':
			topics='roadmap'
		if (topics=='rl treff'
			or topics=='rl-treffen'
			or topics=='rl treff foto contest'):
			topics='irl'
		if (topics=='ats patch' or topics=='testserver'):
			topics='ats'
		if (topics=='e3 news' or topics=='messe'):
			topics='convention'
		if (topics=='jubil\xe4um' or topics=='ryzom'):
			topics='ryzom'
		if (topics=='' or topics=='g\xe9n\xe9ral'):
			topics='general'
		if topics=='interview':
			topics='article'
		if topics=='press release':
			topics='press-release'

		#creation du topics s'il n'existe pas
		try:
			addTopic(self, newblog,topics,topics, blogTopics)
		except:
			pass

		date=year+"-"+month+"-"+day+" "+hour+":"+min+":00"
		excerpt=''
		desc=''
		text = body+more
		
		#gestion du codage latin, utf et windows-1252 (zarb)
		try:
			title=title.replace('\xc2','').decode('cp1252').encode('utf')
		except:
			try:
				title=title.decode('latin')
			except:
				pass

		new_id = qtool.idFromTitle(title, 'WeblogEntry', newblog)
		
		#dictionnaire ancienneid:nouveauchemin
		if len(day)==1:
			day='0'+day
		chemin= "/ryzom/" + archive_path + year + "/" + month + "/" + day + "/" + new_id
		link[str(id)]=chemin
		#correction des liens
		text=rewriteLink(link,text,id)

		try:
			new_entry=createDraft(self,target,new_id,title,desc,date,excerpt,text,topics,lang,author)
		except:
			#link.update({str(id):chemin})
			link[str(id)]=chemin+str(id)
			#correction des liens
			text=rewriteLink(link,text,id)
                        new_id=new_id+str(id)
                        
			new_entry=createDraft(self,target,new_id,title,desc,date,excerpt,text,topics,lang,author)		
		rewrite_rules=rewrite_rules + "\r" + mod_rewrite_rule(self, title,new_id,archive_path,str(year),str(month),str(day))
	return rewrite_rules
