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

	def setTitle(self, value, **kwargs):
		self.getField('title').set(self, value, **kwargs)
		if value:
			try:
				self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())
			except:
                                pass #try to do better than this

	
		
       

	def parseTime(self,date):
   	 return timegm(
      	time.strptime(date.split('GMT')[0], "%Y/%m/%d %H:%M:%S %Z"))     
	
	def get_atys_forums2(self):
		date1=self.parseTime(str(self.getDatestart()))       	 	
		date2=self.parseTime(str(self.getDatearrivee()))
		results=self.qna(start = date1,end = date2)
		tab = []
		for row in results:
			text=str(row[2])
			try:
				text=text.replace('\xc2','').decode('cp1252').encode('utf')
			except:
				try:
					text=text.decode('utf').encode('latin')
				except:
					text=text.decode('latin')
                	tab.append(str(row[0])+' - '+str(row[1])+' - '+text)

		return tab  
        	
        	


#enregisitrements de la classe de Archetypes -cf_init_
registerType(QnA,PROJECTNAME)								
								
