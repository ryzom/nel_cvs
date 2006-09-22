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


RingRankingSchema=BaseFolderSchema.copy()+ Schema(())
 
 
   
class RingRanking(BaseFolder):
	"""Add an Ring Ranking Folder"""

	security = ClassSecurityInfo()
	schema = RingRankingSchema
	archetype_name = meta_type = 'RingRanking'
	allowed_content_types = ['AuthorsRanking',
				 'OwnersRanking',
				 'ScenarioRanking',]
	_at_rename_after_creation = True

#	actions = (
#		{ 'id': 'view',
#		'name': 'view',
#		'action': 'string:${object_url}/RingRanking_view',
#		'permissions': (CMFCorePermissions.View,)
#		},
#	)

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'initializeArchetype')
	def initializeArchetype(self, **kwargs):
		"""use automatically at creation"""
		BaseFolder.initializeArchetype(self, **kwargs)

		for name in self.allowed_content_types:
			# Create a 'name' category
			if not hasattr(self.aq_inner.aq_explicit, name):
				self.invokeFactory(name,id=name)
				obj = getattr(self.aq_inner.aq_explicit, name)
				obj.setTitle(name)

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'list_content_types')
	def list_content_types(self):
		"""return the list of authorized content"""
		return self.allowed_content_types

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'updateAllRank')
	def updateAllRank(self):
		"""update all ranking in the content"""
		result = []
		for name in self.allowed_content_types:
			obj = getattr(self.aq_inner.aq_explicit, name)
			result.append(obj.update())
		return result
		


registerType(RingRanking,PROJECTNAME)								
								
