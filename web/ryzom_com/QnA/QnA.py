#import d'archetypes
from Products.Archetypes.public import *
from Products.CMFCore import CMFCorePermissions
#import de zope
from Products.Archetypes.Marshall import PrimaryFieldMarshaller
from AccessControl import ClassSecurityInfo

import DateTime
#import des valeurs globales

from config import PROJECTNAME

	
#declaration de la gestion des permission/des securitÃ©s dans zope
	
security =ClassSecurityInfo()
		
#implimentation des classe dont on hÃ©rite
__implements__=(BaseContent.__implements__)

#dÃ©fini le shÃ©ma
 
QnASchema=BaseSchema.copy()+ Schema((
         StringField('datestart',
		default='1146163496',
		required=True,
		widget=StringWidget(description="date de dÃ©part ",label="Date",)
	),
        StringField('datearrivee',
		default='1147514161',
		required=True,
		widget=StringWidget(description="date d'arrivÃ©e",label="Date",)
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
	
        def getArchiveUrl(self):
        # FIXME: Is this needed anymore?
       	 year = str(DateTime.DateTime(str(self.getEffectiveDate())).year())
       	 month = DateTime.DateTime(str(self.getEffectiveDate())).mm()
       	 day = DateTime.DateTime(str(self.getEffectiveDate())).dd()
       	 path = 'archive/' + year + '/' + month + '/' + day + '/' + self.id
       	 return path
       
	
	def get_atys_forums2(self):
		date1=int(self.getDatestart())
		date2=int(self.getDatearrivee())
		try :
        		import MySQLdb
       		except :
        		 raise UserError, _('The connection with MySQL failed')

        	connection = MySQLdb.connect(host="localhost",user='root',  db="atys_forums2")	

        	cursor = connection.cursor()
        	cursor.execute("""select distinct dateline,username from post where dateline between %(date1)i and %(date2)i;""" % {'date1': date1,'date2': date2} )

	        results = cursor.fetchall()
		tab = []
		for row in results:
			tab.append(str(row[0])+' - '+str(row[1]))
		return tab
        	
        	


#enregisitrements de la classe de Archetypes -cf_init_
registerType(QnA,PROJECTNAME)								
								
