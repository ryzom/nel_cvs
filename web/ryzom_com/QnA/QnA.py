#import d'archetypes
from Products.Archetypes.public import *
from Products.CMFCore import CMFCorePermissions
#import de zope
from Products.Archetypes.Marshall import PrimaryFieldMarshaller
from AccessControl import ClassSecurityInfo
from calendar import timegm
import DateTime
import time
from Products.CMFCore.DirectoryView import addDirectoryViews
#import des valeurs globales
import re
from config import *
from OFS.Folder import Folder
	
#declaration de la gestion des permission/des securitÃ©s dans zope
	
security =ClassSecurityInfo()
		
#implimentation des classe dont on hÃ©rite
__implements__=(BaseContent.__implements__)

#dÃ©fini le shÃ©ma
 
QnASchema=BaseSchema.copy()+ Schema((
         DateTimeField('datestart',
		required=True,
		widget=CalendarWidget(description="date de dÃ©part ",label="Date",)
	),
        DateTimeField('datearrivee',
		required=True,
		widget=CalendarWidget(description="date d'arrivÃ©e",label="Date",)
	),
	TextField('text',
		searchable=1,
		default_output_type='text/restructured',
		widget=TextAreaWidget(		
		description="not visible in the final version" ,
		visible={'edit':'hidden', 'view':'visible'},
		)
        ),
	
	TextField('description',
		searchable=1,
		widget=TextAreaWidget(description="Enter a little description of the content link",)
	),
        LinesField('choice',
              default = 'general',
              widget=MultiSelectionWidget(label='Categories',format='checkbox'),
              vocabulary='get_atys_forums2',
             
          schemata='configuration'
        ),
        
        
))
 
 
   
class QnA(BaseContent):

	"""Add an QnA Document"""
	schema = QnASchema
	#def __init__(self,BaseContent):
	#	addDirectoryViews(self, 'sql', GLOBALS)
        #actions = (
	#	{ 'id': 'edit',
	#	'name': 'edit',
	#	'action': 'string:${object_url}/qna_edit',
	#	'permissions': (CMFCorePermissions.ModifyPortalContent,)
	#	},
	#)
        actions = (
		{ 'id': 'view',
		'name': 'view',
		'action': 'string:${object_url}/qna_view'
		},
	)
	def setTitle(self, value, **kwargs):
		self.getField('title').set(self, value, **kwargs)
		if value:
			try:
				self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())
			except:
                                pass #try to do better than this

	def setText(self,value,**kwargs):
		tab=self.getChoice()
		text=''
		
		for post_joined in tab:
			post_splitted = post_joined.split('|', )
			
			try:
				post_author = post_splitted[1]
				post_text   = post_splitted[2]
				post_id     = post_splitted[3]

				text += post_text
				text += '<p>-- %s <a href="http://ryzom.com/forum/showthread.php?p=%d#post%d">[ Link ]</a></p><hr />' % (post_author, post_text, post_id)
			except IndexError:
				text += "Error - '%s'" % str(post_splitted)

		self.getField('text').set(self,text,**kwargs)
		
       

	def parseTime(self,date):
   	 return timegm(
      	time.strptime(date.split('GMT')[0], "%Y/%m/%d %H:%M:%S %Z"))     
	
	def get_atys_forums2(self):
		date1=self.parseTime(str(self.getDatestart()))       	 	
		date2=self.parseTime(str(self.getDatearrivee()))
		results=self.qna(start = date1,end = date2)
		tab=[]
		for row in results:                        
			post_i  = str(row[0])
			post_author = str(row[1])
			post_text   = str(row[2])
			#post_id     = int(row[3])
			try:
				post_id = str(row[3])
			except:
				post_id="-1"
			try:
				post_text=post_text.replace('\xc2','').decode('cp1252').encode('utf')
			except:
				try:
					post_text=post_text.decode('utf').encode('latin')
				except:
					post_text=post_text.decode('latin')
			post_text=self.filtertext(post_text)
                	#tab.append(str(row[0])+' - '+str(row[1])+' - '+post_text)
                	#tab.append('|'.join((post_date, post_author, post_text, post_id)))
			tab.append(join((post_date, post_author, post_text, post_id)))
		return tab

	def filtertext(self,text):		
	# Replace string
                newstr = re.sub('\[QUOTE=.*?\]','<div class="news_quote">',text)
		newstr = re.sub('\[/QUOTE\]','</div>',newstr)

		urlindex=newstr.find('[URL=')+5
		urlend=newstr[urlindex:].find(']')
		url=newstr[urlindex:].split(']',1)[0]
		newstr = re.sub('\[URL=.*?\]','<a href="'+url+'">',newstr)

		urlindex=newstr.find('[url=')+5
		urlend=newstr[urlindex:].find(']')
		url=newstr[urlindex:].split(']',1)[0]
		newstr = re.sub('\[url=.*?\]','<a href="'+url+'">',newstr)


		colorindex=newstr.find('[color=')+6
		colorend=newstr[colorindex:].find(']')
		color=newstr[colorindex:].split(']',1)[0]
		newstr = re.sub('\[color=.*?\]','<span style="color"'+color+'">',newstr)
		
		COLORindex=newstr.find('[COLOR=')+6
		COLORend=newstr[COLORindex:].find(']')
		COLOR=newstr[COLORindex:].split(']',1)[0]
		newstr = re.sub('\[COLOR=.*?\]','<span style="COLOR"'+COLOR+'">',newstr)
		
		fontindex=newstr.find('[font=')+5
		fontend=newstr[urlindex:].find(']')
		font=newstr[fontindex:].split(']',1)[0]
		newstr = re.sub('\[font=.*?\]','<font-family'+font+'">',newstr)

		sizeindex=newstr.find('[size=')+5
		sizeend=newstr[sizeindex:].find(']')
		size=newstr[sizeindex:].split(']',1)[0]
		newstr = re.sub('\[size=.*?\]','<font-size'+size+'">',newstr)

		listindex=newstr.find('[size=')+5
		listend=newstr[listindex:].find(']')
		list=newstr[listindex:].split(']',1)[0]
		
		listindex=newstr.find('[size=')+5
		listend=newstr[listindex:].find(']')
		list=newstr[listindex:].split(']',1)[0]

		newstr = re.sub('\[size=.*?\]','<list'+size+'">',newstr)
		newstr = re.sub('\[i\]','<i>',newstr)
		newstr = re.sub('\[/i\]','</i>',newstr)
		newstr = re.sub('\[b\]','<b>',newstr)
		newstr = re.sub('\[/b\]','</b>',newstr)
		newstr = re.sub('\[IMG\]','<IMG>',newstr)
		newstr = re.sub('\[/IMG\]','</IMG>',newstr)
		newstr = re.sub('\[center\]','<center>',newstr)
		newstr = re.sub('\[/center\]','</center>',newstr)
		newstr = re.sub('\[/color\]','</color>',newstr)
		newstr = re.sub('\[/size\]','</size>',newstr) 
		newstr = re.sub('\[/url\]','</a>',newstr)
		newstr = re.sub('\[url\]','<a>',newstr)
		newstr = re.sub('\[URL\]','<a>',newstr)
		newstr = re.sub('\[/URL\]','</a>',newstr)
		newstr = re.sub('\[u\]','<h1>',newstr)
		newstr = re.sub('\[/u\]','</h1>',newstr)
		newstr = re.sub('\[U\]','<h1>',newstr)
		newstr = re.sub('\[/U\]','</h1>',newstr)
 		newstr = re.sub('\[email\]','<email>',newstr)
		newstr = re.sub('\[/email\]','</email>',newstr)
		newstr = re.sub('\[img\]','<img>',newstr)
		newstr = re.sub('\[/img\]','</img>',newstr)
		newstr = re.sub('\[list\]','<list>',newstr)
		newstr = re.sub('\[/list\]','</elist>',newstr)
		newstr = re.sub('\[/color\]','</span >',newstr)
		newstr = re.sub('\[/COLOR\]','</span >',newstr)
		newstr = re.sub('\[QUOTE\]','<QUOTE>',newstr)
		newstr = re.sub('\[/QUOTE\]','</QUOTE>',newstr)
		newstr = re.sub('\[edit\]','<edit>',newstr)
		newstr = re.sub('\[/edit\]','</edit>',newstr)
		newstr = re.sub('\[quote\]','<div class="news_quote">',newstr)
		newstr = re.sub('\[/quote\]','</div>',newstr)
		newstr = re.sub('\[/font\]','</font-family >',newstr)

		newstr = re.sub('\n','<br />',newstr)

		return newstr
		
        	
        	


#enregisitrements de la classe de Archetypes -cf_init_
registerType(QnA,PROJECTNAME)								
								
