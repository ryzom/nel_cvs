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

#Ajouter un formulaire pour modifier le titre des objets generer a la crÃ©ation.

AnikiSchema = BaseFolderSchema.copy()+ Schema((
	TextField('description',
		searchable=False,
		widget=TextAreaWidget(description="A little description of the campaign",)
	),
))

class Aniki(BaseFolder):
	"""Aniki handles the Surveillant wich watch all the ads"""

	security = ClassSecurityInfo()
	schema = AnikiSchema
	archetype_name = meta_type = 'Aniki'
	allowed_content_types = ['Surveillant',]
	_at_rename_after_creation = True # renomme le titre en identificateur si possible

	actions = (
		{ 'id': 'view',
		'name': 'view',
		'action': 'string:${object_url}/Aniki_view',
		'permissions': (CMFCorePermissions.View,)
		},
	)

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'initializeArchetype')
	def initializeArchetype(self, **kwargs):
		"""use automatically at creation"""
		BaseFolder.initializeArchetype(self, **kwargs)

	security.declareProtected(CMFCorePermissions.View, 'redirect')
	def redirect(self, idPub):
		"""This method will redirect to the URL"""
		path = '/'.join(self.getPhysicalPath())
		results = self.portal_catalog(
			meta_type=['Surveillant',],
			path={'query':path, 'level': 0},
			id=idPub,
			)
		results[0].getObject().goto()

registerType(Aniki, PROJECTNAME)