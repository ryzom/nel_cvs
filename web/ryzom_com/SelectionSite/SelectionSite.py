try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *
#from Products.Archetypes.Marshall import RFC822Marshaller
#from Products.Archetypes.Marshall import PrimaryFieldMarshaller

from Products.CMFCore import CMFCorePermissions

#from Products.ATContentTypes.configuration import zconf
#from Products.ATContentTypes.lib.imagetransform import ATCTImageTransform

#from Products.validation.config import validation
#from Products.validation.validators.SupplValidators import MaxSizeValidator
#from Products.validation import V_REQUIRED
#validation.register(MaxSizeValidator('checkImageMaxSize',maxsize=zconf.ATImage.max_file_size))


#from FanVisitg1 import * 
from config import PROJECTNAME


SelectionSchema = BaseSchema.copy() + Schema((   
#	StringField('Site',
#		searchable=1,
#		required=1,
#		index = 'FieldIndex',
#		widget=StringWidget(label='site/website',
#			description='description of the site',
#		),
#	),	      
#	ImageField('counter_selectionsite',
#		required=0,
#		default ='side_bg.png',
#		default_method = 'setImage'#	 	mutator = 'setImage',
#		default_output_type='image/png',
#		allowable_content_types=('image/*',),
#		pil_quality = zconf.pil_config.quality,
#	 	pil_resize_algo = zconf.pil_config.resize_algo,
#		max_size = zconf.ATImage.max_image_dimension,
#		sizes= {'large'   : (768, 768),
#			'preview' : (400, 400),
#			'mini'    : (200, 200),
#			'thumb'   : (128, 128),
#			'tile'    :  (64, 64),
#			'icon'    :  (32, 32),
#			'listing' :  (16, 16),
#		},	       
#		widget=ImageWidget(description='counter'),
#	),
#	ImageField('topicImage',
#		required=0,
#		default ='default-topic-icon.png',
#		sizes={'64' : (64,64),},
#		widget=ImageWidget(label="Image",
#			description="An image for your topic. The image should be 64x64.  If the image is larger it will be scaled down automatically to fit within a 64x64 square."),
#	),
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
	StringField('category',
		searchable=1,
		required=0,
		vocabulary=['News','Tutorial'],            
		widget=SelectionWidget(description='select your category?'),
		visible= {"edit": "hidden", "view": "hidden"},
	),
	StringField('thirtysum',           
		widget=IntegerWidget(description=''),
		visible= {"edit": "hidden", "view": "hidden"},
	),
	StringField('othersum',        
		widget=IntegerWidget(description=''),
		visible= {"edit": "hidden", "view": "hidden"},
	),
	),
#	marshall=PrimaryFieldMarshaller(),
)
  
    

class SelectionSite(BaseContent):
	"""Selection Site"""
#	counter_selectionsite = ''
#	default_counter_image='side_bg.png',
#	topicImage = ''
#	default_topic_image='default-topic-icon.png',
    
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
    
#	schema["title"].required = 0
#	schema["title"].widget.visible = {"edit": "invisible", "view": "invisible"}
#	schema["counter_selectionsite"].widget.visible = {"edit": "hidden", "view":"hidden"}

	def __init__(self, oid, **kwargs):
		self.listVisit = {}
		BaseContent.__init__(self, oid, **kwargs)

	def thirtyVisit(self):
		return self.listVisit[-30:0]
    
	def updateDico(self,dico):
		#mise a jour du dictionnaire
		self.listVisit.update(dico)
		#on recupere la list des clefs trie
		k = self.listVisit.keys()
		k.sort()
		v = []
		sum = 0
		#v contient les visites ordonnee par date
		for i in k:
			v.append(self.listVisit[i])		
		for i in v[-30:]:
			sum += i
		self.setThirtysum(sum)
		for i in v[0:-30]:
			sum += i
		self.setOtherSum(sum)

	def getVisit(self):
		return self.listVisit
		
#	def getSumVisit(self):
#		return self.getThirtysum+self.getOtherSum

#	def setSite(self, value):
#		self.Site = value
#		return self.setTitle(value)

	def setTitle(self, value, **kwargs):
		self.getField('title').set(self, value, **kwargs)
		if value:
			try:
				self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())
			except:
				pass #try to do better than this

registerType(SelectionSite, PROJECTNAME)
