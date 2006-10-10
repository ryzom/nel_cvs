# -*- coding: utf-8 -*-
#import python
from calendar import timegm
from DateTime import DateTime
from string import join
import time
import re

#import zope/archetype
from AccessControl import ClassSecurityInfo
from Products.CMFCore.utils import getToolByName
from Products.CMFCore import CMFCorePermissions
from Products.ATContentTypes.permission import ChangeEvents
try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *

#import de fonction du produit
from config import *


QnASchema=BaseSchema.copy()+ Schema((
	DateTimeField('dateStart',
		default=DateTime(),
		required=True,
		searchable=False,
		widget=CalendarWidget(description="date de dÃ©part",label="Date",)
	),
	DateTimeField('dateEnd',
		default=DateTime(),
		required=True,
		searchable=False,
		widget=CalendarWidget(description="date d'arrivÃ©e",label="Date",)
	),	
	TextField('description',
		searchable=False,
		widget=TextAreaWidget(description="Enter a little description of the content link",)
	),
	TextField('filter',
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

	security = ClassSecurityInfo()
	schema = QnASchema
	archetype_name = "QnA"
	meta_type = 'QnA'
	default_view  = 'qna_view'
	immediate_view = 'qna_view'
	_at_rename_after_creation = True

	actions = (
		{ 'id': 'view',
		'name': 'view',
		'action': 'string:${object_url}/qna_view',
		'permissions': (CMFCorePermissions.View,)
		},
		{ 'id': 'edit',
		'name': 'edit',
		'action': 'string:${object_url}/qna_edit_request_form',
		'permissions': (CMFCorePermissions.ModifyPortalContent,)
		},
		{ 'id': 'choice',
		'name': 'choice',
		'action': 'string:${object_url}/qna_edit_choice_form',
		'permissions': (CMFCorePermissions.ModifyPortalContent,)
		},
	)


	#PostList and accessor & mutator
	PostList={}

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'getPostList')
	def getPostList(self):
		"""return the postlist"""
		return self.PostList

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setPostList')
	def setPostList(self,dico):
		"""set the PostList"""
		self.PostList=dico

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setFilter')
	def setFilter(self, value, **kwargs):
		"""set filter fields"""
		if not value:
			self.getField('filter').set(self, str(self.getUsersOfficials()), **kwargs)
		else:
			self.getField('filter').set(self, value, **kwargs)

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'generate_text')
	def generate_text(self,post):
		"""write text from post"""
		newtext=''
		newtext+=self.filtertext(post[2])
		newtext+='<p id="qna_author_link">-- %s <a href="http://ryzom.com/forum/showthread.php?p=%s#post%s">[ Link ]</a></p><hr />' % (post[1], post[3], post[3])
		return newtext
		
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'parseTime')
	def parseTime(self,date):
		"""convert date to timestamp"""
		cleandate = date.split(' ')[0]+date.split(' ')[1]
		result = timegm(time.strptime(cleandate, "%Y/%m/%d %H:%M:%S"))
		return result

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'getGroupUsers')
	def getGroupUsers(self,groupid):
		"""return users in group"""
		acl_users = getToolByName(self,'acl_users')
    		users=acl_users.getUsers()
   		prefix=acl_users.getGroupPrefix()
   		avail=[]
   		for user in users:
      			for group in user.getGroups():
          			 if groupid==group or prefix+groupid==group:
              				 avail.append(str(user))
   		return avail

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'getUsersOfficials')
	def getUsersOfficials(self):
		"""return Official's users"""
		return join(self.getGroupUsers('Officials'),';')

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'getSQLPostList')
	def getSQLPostList(self):
		"""return a the results of the sql request"""
		#on convertie les dates en timestamp
		date1=self.parseTime(str(self.getDateStart()))       	 	
		date2=self.parseTime(str(self.getDateEnd()))
		#la liste des utilisateur a rechercher par defaut
		OfficialsNames = self.getFilter().split(';')
		#execute la requete sql
		results=self.qna(username = OfficialsNames, start = date1, end = date2)
		return results

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'createPostList')
	def createPostList(self):
		"""return the PostList in a dictionnary"""
		SQLPostList=self.getSQLPostList()
		PostList={}
		i=0
		for row in SQLPostList:
			i+=1
			post_id  = str(row[0])
			post_date = str(row[1])
			post_author = str(row[2])
			post_text   = str(row[3])
			post_title  = str(row[4])
			#conversion du texte
			try:
				post_text=post_text.replace('\xc2','').decode('cp1252').encode('utf')
			except:
				try:
					post_text=post_text.decode('utf').encode('latin')
				except:
					post_text=post_text.decode('latin')
			#ajout des donnÃ©es dans le dictionnaire
			newtext = self.generate_text([post_date,post_author,post_text,post_id])
			PostList.update({i:[post_date,post_author,newtext,post_id,post_title]})
		#return PostList
		self.setPostList(PostList)

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'filtertext')
	def filtertext(self,text):		
		"""(poorly) translate bbcode to html code"""
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

registerType(QnA,PROJECTNAME)								
								
