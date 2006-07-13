# -*- coding: utf-8 -*-
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
	"""Add a Patch Note"""

	schema = UrlPatchNoteSchema
	meta_type = portal_type = 'UrlPatchNote'
	global_allow = 0

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
			try:
				self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())
			except:
                                pass #try to do better than this
                                

	def setText(self, value, **kwargs):		
		lang=self.getLang()
		ident=self.getIdent()
		url = 'http://www.ryzom.com/export_rn.php?lang='+lang+'&archive=0&id='+ident
		text = urllib2.urlopen(url).read()
		#on nettoie le style
		text=re.sub('<STYLE>\\n*\\t*.*?</style>','',text)
		#on nettoie les tags html non autorise
		text=strip(text)
		#insert unicode for french and deutch content
		if lang=='fr' or lang =='de':
			try:
				self.getField('text').set(self, text.replace('\xc2','').decode('cp1252').encode('utf'), **kwargs)
			except:
				try:
					self.getField('text').set(self, text.decode('utf').encode('latin'), **kwargs)
				except:
					self.getField('text').set(self, text.decode('latin'), **kwargs)
		else:
			try:
				self.getField('text').set(self, text.replace('\xc2','').decode('cp1252').encode('utf'), **kwargs)
			except:
				self.getField('text').set(self, text, **kwargs)

registerType(UrlPatchNote,PROJECTNAME)
