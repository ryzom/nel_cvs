# -*- coding: utf-8 -*-

#import zope/archetype
from AccessControl import ClassSecurityInfo
from Products.CMFCore.utils import getToolByName
from Products.CMFCore import CMFCorePermissions

try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *

#import de fonction du produit
from config import *

surveillantSchema=BaseSchema.copy()+ Schema((
	TextField('description',
		searchable=False,
		widget=TextAreaWidget(description="A little description of the Ad",)
	),
	StringField('urlRedirection',
		searchable=False,
		widget=StringWidget(description="Where the link should go",)
	),
),)
 
surveillantSchema['id'].widget.visible = { 'view': 'visible', 'edit': 'visible',}

class Surveillant(BaseContent):
	"""This object makes the redirection for one ad"""
	security = ClassSecurityInfo()
	schema = surveillantSchema
	meta_type = portal_type = 'Surveillant'
	global_allow = 0
	_at_rename_after_creation = True # renomme le titre en identificateur si possible

	iterateur = int(0) # objet nous permmettant le decompte de click

	actions = (
#		{ 'id': 'view',
#		'name': 'View',
#		'action': 'string:${object_url}/Surveillant_view',
#		'permissions': (CMFCorePermissions.View,)
#		},
	)
	
	#def __init__(self, id):
	#	self.iterateur = 0
	
	security.declareProtected(CMFCorePermissions.View, 'getIterateur')
	def getIterateur(self):
		"""get the number of access"""
		return self.iterateur
	
	def goto(self,REQUEST):
		"""Go the url"""
		self.iterateur += 1
		self.redirect(REQUEST)
		
	def redirect(self,REQUEST):
		"""Makes a redirection"""
#		request = container.REQUEST
#		RESPONSE =  request.RESPONSE
#		RESPONSE.redirect(self.getUrlRedirection())
		REQUEST['RESPONSE'].redirect(self.getUrlRedirection())
		
registerType(Surveillant, PROJECTNAME)
