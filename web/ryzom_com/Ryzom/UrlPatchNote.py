import urllib2
import re
try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *
from Products.CMFCore import CMFCorePermissions
from config import PROJECTNAME

#strip tag support for ryzom site
from Strip import *

UrlPatchNoteSchema = BaseSchema.copy() + Schema((
	StringField('ident',
		default='151',
		required=True,
		widget=StringWidget(description="Patch Note ID ",label="ID",)
	),
	StringField('lang',
		default='en',
		required=True,
		widget=StringWidget(description="PatchNote Langage",)
	),	
	TextField('description',
		searchable=1,
		widget=TextAreaWidget(description="Enter a little description of the content link",)
	),
	TextField('text',
		searchable=1,
		default_output_type='text/restructured',
		widget=TextAreaWidget(		
		description="not visible in the final version" ,
		visible={'edit':'hidden', 'view':'visible'},
		)
        ),
),)	


class UrlPatchNote(BaseContent):
	"""Add an Url Document"""
	schema = UrlPatchNoteSchema

	actions = (
		{ 'id': 'view',
		'name': 'View',
		'action': 'string:${object_url}/urlpatchnote_view',
		'permissions': (CMFCorePermissions.View,)
		},
	)

	def setTitle(self, value, **kwargs):
		self.getField('title').set(self, value, **kwargs)
		if value:
			weblog = self.quills_tool.getParentWeblog(self)
			self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())

	def setText(self, value, **kwargs):		
		lang=self.getLang()
		ident=self.getIdent()
		url = 'http://www.ryzom.com/export_rn.php?lang='+lang+'&archive=0&id='+ident
		text = urllib2.urlopen(url).read()
		#on nettoie le style
		text=re.sub('<STYLE>\\n*\\t*.*?</style>','',text)
		#on nettoie les tags html non autorise
		text=strip(text)
		self.getField('text').set(self, text, **kwargs)

registerType(UrlPatchNote,PROJECTNAME)
