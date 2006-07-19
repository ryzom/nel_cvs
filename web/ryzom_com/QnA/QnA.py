# -*- coding: utf-8 -*-
#import python
from calendar import timegm
from DateTime import DateTime
from string import join
import DateTime
import time
import re
from Products.CMFCore.utils import getToolByName

#import zope/archetype
from Products.CMFCore import CMFCorePermissions
from Products.ATContentTypes.permission import ChangeEvents
try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *

#import de fonction du produit
from config import *

#dÃ©fini le shÃ©ma 
QnASchema=BaseSchema.copy()+ Schema((
	DateTimeField('datestart',
		required=True,
		searchable=False,
		widget=CalendarWidget(description="date de dÃ©part ",label="Date",)
	),
	DateTimeField('datearrivee',
		required=True,
		searchable=False,
		widget=CalendarWidget(description="date d'arrivÃ©e",label="Date",)
	),	
	TextField('description',
		searchable=False,
		widget=TextAreaWidget(description="Enter a little description of the content link",)
	),
	TextField('filtrage',
		searchable=False,
		widget=TextAreaWidget(description="Enter a les noms des auteurs Ã  chercher",)
	),
	LinesField('choice',
		searchable=False,
		default_output_type='text/html',	
		widget=MultiSelectionWidget(label='Categories',format='checkbox'),
		vocabulary='get_atys_forums2',             
		schemata='configuration',
	),
	TextField('text',
		searchable=False,
		default_output_type='text/html',
		widget=TextAreaWidget(description="edit the choice",visible={'edit':'hidden', 'view':'visible'},),
		schemata='configuration',
    ),
),)
 
 
   
class QnA(BaseContent):

	"""Add an QnA Document"""
	schema = QnASchema

	archetype_name = "QnA"
	meta_type = 'QnA'
	default_view  = 'qna_view'
	immediate_view = 'qna_view'

	actions = (
		{ 'id': 'view',
		'name': 'view',
		'action': 'string:${object_url}/qna_view',
		'permissions': (CMFCorePermissions.View,)
		},
	#	{ 'id': 'edit',
	#	'name': 'edit',
	#	'action': 'string:${object_url}/qna_edit',
	#	'permissions': (CMFCorePermissions.ModifyPortalContent,)
	#	},
	)

	#si le champ "filtrage" est vide il est remplie par le nom des utilisateurs du groupe official
	def setFiltrage(self, value, **kwargs):
		if not value:
			self.getField('filtrage').set(self, str(self.getUsersOfficials()), **kwargs)
		else:
			self.getField('filtrage').set(self, value, **kwargs)

	def setTitle(self, value, **kwargs):
		self.getField('title').set(self, value, **kwargs)
		if value:
			try:
				self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())
			except:
				pass #try to do better than this


	def setText(self,value,**kwargs):
		self.getField('text').set(self,self.generate_text(),**kwargs)		



	def generate_text(self):
		tab=self.getChoice()
		text=''		
		for post_joined in tab:
			post_splitted = post_joined.split('|-|')
			try:
				post_author = post_splitted[1]
				post_text   = post_splitted[2]
				post_id     = post_splitted[3]
				text += post_text
				text += '<p id="qna_author_link">-- %s <a href="http://ryzom.com/forum/showthread.php?p=%s#post%s">[ Link ]</a></p><hr />' % (post_author, post_id, post_id)
			except IndexError:
				text += "Error - " + str(post_splitted)
		return text

	#cette fonction est appelle lors de la visualisation du qna et corrige le champ text s'il n'est pas correct
#	def auto_correction(self):
#		text = self.getText()
#		if not text:
#			self.setText('truc')
#		return text

	
	#def getText(self):
	#	text = self.getField('text').get(self,value,**kwargs)
	#	if not text or text=='':
	#		self.setText('truc')
	#	return text
			
		
	
	#script de validation 	
	#def validation_date(self,date):
	#	 return self.parseTime(date) <= self.parseTime(DateTime())   
   

	#convertit une date en timestamp
	#TODO: gerer les heures du type 00:00
	def parseTime(self,date):		
		try:
			result = timegm(time.strptime(date.split('GMT')[0], "%Y/%m/%d %H:%M:%S %Z"))
   		except:
			result = timegm(time.strptime(date.split('GMT')[0], "%Y/%m/%d 00:00:00 %Z"))
		return result

	def parseName(self,name):
		n=name.split()
		tab=[]
		for i in n:
			tab.append("'"+i+"'")
		return join(tab,',')

	#renvoie la liste des utilisateurs d'un groupe
	def getGroupUsers(self,groupid):   
		acl_users = getToolByName(self,'acl_users')
    		users=acl_users.getUsers()
   		prefix=acl_users.getGroupPrefix()
   		avail=[]
   		for user in users:
      			for group in user.getGroups():
          			 if groupid==group or prefix+groupid==group:
              				 avail.append(str(user))
   		return avail

	#renvoie la liste du groupe officials
	def getUsersOfficials(self):
		return join(self.getGroupUsers('Officials'),' ')


	def get_atys_forums2(self):
		date1=self.parseTime(str(self.getDatestart()))       	 	
		date2=self.parseTime(str(self.getDatearrivee()))
		OfficialsNames = self.getFiltrage().split()
		results=self.qna(username = OfficialsNames, start = date1, end = date2)
		tab=[]
		for row in results:                        
			post_id  = str(row[0])
			post_date = str(row[1])
			post_author = str(row[2])
			post_text   = str(row[3])

			try:
				post_text=post_text.replace('\xc2','').decode('cp1252').encode('utf')
			except:
				try:
					post_text=post_text.decode('utf').encode('latin')
				except:
					post_text=post_text.decode('latin')
			post_text=self.filtertext(post_text)
			#tab.append((post_date,post_author,post_text,post_id))
			tab.append('|-|'.join((post_date,post_author,post_text,post_id)))
		return tab

	#def get_atys_forums2(self):
	#	result=[]
	#	tab = self.get_atys_forums()
	#	for i in tab:
	#		post_id  = str(i[3])
	#		post_date = str(i[0])
	#		post_author = str(i[1])
	#		post_text   = str(i[2])
	#		result.append(str((post_date,post_author,post_text,post_id))
	#	return result


	def filtertext(self,text):		
		# Replace string
		newstr=text

		quoteindex=newstr.find('[QUOTE=')+7
		quoteend=newstr[quoteindex:].find(']')
		quote=newstr[quoteindex:].split(']',1)[0]
		newstr = re.sub('\[QUOTE=.*?\]','<div class="news_quote"> Originally Posted by '+quote+'<br />',newstr)
		newstr = re.sub('\[/QUOTE\]','</div>',newstr)

		quoteindex=newstr.find('[quote=')+7
		quoteend=newstr[quoteindex:].find(']')
		quote=newstr[quoteindex:].split(']',1)[0]
		newstr = re.sub('\[quote=.*?\]','<div class="news_quote"> Originally Posted by '+quote+'<br />',newstr)
		newstr = re.sub('\[/quote\]','</div>',newstr)

		urlindex=newstr.find('[URL=')+5
		urlend=newstr[urlindex:].find(']')
		url=newstr[urlindex:].split(']',1)[0]
		newstr = re.sub('\[URL=.*?\]','<a href="'+url+'">',newstr)
		newstr = re.sub('\[/URL\]','</a>',newstr)

		urlindex=newstr.find('[url=')+5
		urlend=newstr[urlindex:].find(']')
		url=newstr[urlindex:].split(']',1)[0]
		newstr = re.sub('\[url=.*?\]','<a href="'+url+'">',newstr)
		newstr = re.sub('\[/url\]','</a>',newstr)

		colorindex=newstr.find('[color=')+7
		colorend=newstr[colorindex:].find(']')
		color=newstr[colorindex:].split(']',1)[0]
		newstr = re.sub('\[color=.*?\]','<span style="color:'+color+'">',newstr)
		newstr = re.sub('\[/color\]','</span>',newstr)
		
		colorindex=newstr.find('[COLOR=')+7
		colorend=newstr[colorindex:].find(']')
		color=newstr[colorindex:].split(']',1)[0]
		newstr = re.sub('\[COLOR=.*?\]','<span style="color:'+color+'">',newstr)
		newstr = re.sub('\[/COLOR\]','</span>',newstr)
		
		fontindex=newstr.find('[font=')+6
		fontend=newstr[fontindex:].find(']')
		font=newstr[fontindex:].split(']',1)[0]
		newstr = re.sub('\[font=.*?\]','<span style="font-family:'+font+'">',newstr)
		newstr = re.sub('\[/font\]','</span>',newstr)

		sizeindex=newstr.find('[size=')+6
		sizeend=newstr[sizeindex:].find(']')
		size=newstr[sizeindex:].split(']',1)[0]
		newstr = re.sub('\[size=.*?\]','<span style="font-size:'+size+'">',newstr)
		newstr = re.sub('\[/size\]','</span>',newstr)

		sizeindex=newstr.find('[SIZE=')+6
		sizeend=newstr[sizeindex:].find(']')
		size=newstr[sizeindex:].split(']',1)[0]
		newstr = re.sub('\[SIZE=.*?\]','<span style="font-size:'+size+'">',newstr)
		newstr = re.sub('\[/SIZE\]','</span>',newstr)

		newstr = re.sub('\[i\]','<i>',newstr)
		newstr = re.sub('\[/i\]','</i>',newstr)
		newstr = re.sub('\[I\]','<i>',newstr)
		newstr = re.sub('\[/I\]','</i>',newstr)
		newstr = re.sub('\[b\]','<b>',newstr)
		newstr = re.sub('\[/b\]','</b>',newstr)
		newstr = re.sub('\[B\]','<b>',newstr)
		newstr = re.sub('\[/B\]','</b>',newstr)
		newstr = re.sub('\[IMG\]','<img>',newstr)
		newstr = re.sub('\[/IMG\]','</img>',newstr)
		newstr = re.sub('\[img\]','<img>',newstr)
		newstr = re.sub('\[/img\]','</img>',newstr)
		newstr = re.sub('\[center\]','<center>',newstr)
		newstr = re.sub('\[/center\]','</center>',newstr)
 
		newstr = re.sub('\[url\]','<a>',newstr)
		newstr = re.sub('\[URL\]','<a>',newstr)

		newstr = re.sub('\[u\]','<h1>',newstr)
		newstr = re.sub('\[/u\]','</h1>',newstr)
		newstr = re.sub('\[U\]','<h1>',newstr)
		newstr = re.sub('\[/U\]','</h1>',newstr)
 		newstr = re.sub('\[email\]','<email>',newstr)
		newstr = re.sub('\[/email\]','</email>',newstr)

		newstr = re.sub('\[list\]','<li>',newstr)
		newstr = re.sub('\[/list\]','</li>',newstr)
		newstr = re.sub('\[QUOTE\]','<div class="news_quote">',newstr)
		newstr = re.sub('\[quote\]','<div class="news_quote">',newstr)
		newstr = re.sub('\[edit\]','<edit>',newstr)
		newstr = re.sub('\[/edit\]','</edit>',newstr)


		newstr = re.sub('\n','<br />',newstr)

		return newstr
		
        	
        	


#enregisitrements de la classe de Archetypes -cf_init_
registerType(QnA,PROJECTNAME)								
								
