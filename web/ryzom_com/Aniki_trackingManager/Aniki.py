# -*- coding: utf-8 -*-

#import zope/archetype
from AccessControl import ClassSecurityInfo
from Products.CMFCore.utils import getToolByName
from Products.CMFCore import CMFCorePermissions

from Products.Archetypes.public import *

#import de fonction du produit
from config import *

#Ajouter un formulaire pour modifier le titre des objets generer a la crÃ©ation.

AnikiSchema = BaseFolderSchema.copy()+ Schema((
	TextField('description',
		searchable=False,
		widget=TextAreaWidget(description="A little description of the campaign",)
	),
	LinesField('types',
		searchable=False,
		widget=LinesWidget(description="Enter list of type, one per lines",)
	),
	LinesField('formats',
		searchable=False,
		widget=LinesWidget(description="Enter list of format, one per lines",)
	),
	LinesField('langs',
		searchable=False,
		widget=LinesWidget(description="Enter list of language, one per lines",)
	),
	LinesField('authorized_groups',
		searchable=False,
		widget=LinesWidget(description="Enter a list of authorized group(s)",)
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

	security.declarePublic('redirect')
	def redirect(self, idPub,REQUEST):
		"""This method will redirect to the URL"""
		path = '/'.join(self.getPhysicalPath())
		results = self.portal_catalog(
			meta_type=['Surveillant',],
			path={'query':path, 'level': 0},
			id=idPub,
			)
		for result in results:
			result.getObject().goto(REQUEST)

	security.declareProtected(CMFCorePermissions.View, 'getSurveillants')
	def getSurveillants(self):
		"""This method returns a list of the surveillants"""
		path = '/'.join(self.getPhysicalPath())
		results = self.portal_catalog(
			meta_type=['Surveillant',],
			path={'query':path, 'level': 0},
			sort_on = 'sortable_title',
			)
		return results

	security.declarePublic('isAuthorized')
	def isAuthorized(self,name):
		"""return true is name is include in authorized groups"""
		groups = self.getAuthorized_groups()
		user = self.acl_users.getUserByName(name);
		if user:
			userGroups = user.getGroups()
			for group in groups:
				if 'group_'+group in userGroups:
					return True
		return False


registerType(Aniki, PROJECTNAME)
