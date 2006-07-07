try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *
from Products.CMFCore import CMFCorePermissions
from config import PROJECTNAME


SelectionSchema = BaseSchema.copy() + Schema((   
	TextField('description',
		searchable=1,
		required=0,
		widget=TextAreaWidget(label='Summary Description',
			description='description of the site',
		),              
	),
	TextField('text',
		searchable=1,	               
		widget=EpozWidget(label='Item Description',
			description='description of the site',
		),
		required=1,
		default_output_type='text/html',
		allowable_content_types=('text/plain','text/structured','text/html'),
	),
	StringField("url",
		searchable = 0,
		required = 1,
		widget=StringWidget(label="URL/website",
 			label_msgid="label_url",
			description="A link to the official website",
			description_msgid="desc_url",
			validator="isURL",
		),
	),	      
	ImageField('logo',
		required=1,
		default_output_type='image/jpeg',
		allowable_content_types=('image/*',),          
		widget=ImageWidget(description='upload a logo of the site'),
	),
	LinesField('category',
		searchable=1,
		required=0,
		vocabulary=['News','Tutorial'],            
		widget=SelectionWidget(description='select your category?'),
	),
	LinesField('visit',		
		widget=LinesWidget(description=''),
	),
	IntegerField('thirtysum',
		default = 0,
		widget=IntegerWidget(description=''),
	),
	IntegerField('othersum',
		default = 0,
		widget=IntegerWidget(description=''),
	),
	),
)
  
    

class SelectionSite(BaseContent):
	"""Selection Site"""
	schema = SelectionSchema
	archetype_name = "SelectionSite"
	meta_type = 'SelectionSite'
	default_view  = 'SelectionSite_view'
	immediate_view = 'SelectionSite_view'

	actions = (
	{
		'id'          : 'view',
		'name'        : 'View',
 		'action'      : 'string:${object_url}/SelectionSite_view',
 		'permissions' : (CMFCorePermissions.View,)
	},
#	{
#		'id'          : 'edit',
#		'name'        : 'Edit',
#		'action'      : 'string:${object_url}/SelectionSite_edit',
#		'permissions' : (CMFCorePermissions.ModifyPortalContent,),
#	},
	)

	def setTitle(self, value, **kwargs):
		self.getField('title').set(self, value, **kwargs)
		if value:
			try:
				self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())
			except:
				pass #try to do better than this
	def toto(self):
		return 'toto'

	def addVisit(self,dico):
		visit = self.getVisit()
		current_visit = {}
		for line in visit:
			t=line.split(':')
			key=t[0]
			value=t[1]
			current_visit.update({key:value})
		current_visit.update(dico)

		listkey = current_visit.keys()
		listkey.sort()
		updated_visit=[]
		for key in listkey:
			updated_visit.append(str(key)+':'+str(current_visit[key]))
		self.setVisit(updated_visit)


registerType(SelectionSite, PROJECTNAME)
