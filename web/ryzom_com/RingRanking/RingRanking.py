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

RingRankingSchema=BaseFolderSchema.copy()+ Schema(())
 
 
   
class RingRanking(BaseFolder):
	"""Add an Ring Ranking Folder"""

	security = ClassSecurityInfo()
	schema = RingRankingSchema
	archetype_name = meta_type = 'RingRanking'
	allowed_content_types = ['AuthorsRanking',
				 'OwnersRanking',
				 'ScenarioRanking',
				 'AMRanking',]
	_at_rename_after_creation = True

	#name (title & id) of the content created
	content = ['AMRanking',
		   'AuthorRanking',
		   'ScenarioMasterlessRanking',
		   'ScenarioMasteredRanking',]


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

#		for name in self.allowed_content_types:
#			# Create a 'name' category
#			if not hasattr(self.aq_inner.aq_explicit, name):
#				self.invokeFactory(name,id=name)
#				obj = getattr(self.aq_inner.aq_explicit, name)
#				obj.setTitle(name)

		if not hasattr(self.aq_inner.aq_explicit, 'ScenarioMasteredRanking'):
			self.invokeFactory('ScenarioRanking',id='ScenarioMasteredRanking')
			obj = getattr(self.aq_inner.aq_explicit, 'ScenarioMasteredRanking')
			obj.setTitle('ScenarioMasteredRanking')
			obj.setMaster(True)

		if not hasattr(self.aq_inner.aq_explicit, 'ScenarioMasterlessRanking'):
			self.invokeFactory('ScenarioRanking',id='ScenarioMasterlessRanking')
			obj = getattr(self.aq_inner.aq_explicit, 'ScenarioMasterlessRanking')
			obj.setTitle('ScenarioMasterlessRanking')
			obj.setMaster(False)

		if not hasattr(self.aq_inner.aq_explicit, 'AuthorsRanking'):
			self.invokeFactory('AuthorsRanking',id='AuthorsRanking')
			obj = getattr(self.aq_inner.aq_explicit, 'AuthorsRanking')
			obj.setTitle('AuthorsRanking')
			obj.setMaster(False)

		if not hasattr(self.aq_inner.aq_explicit, 'AMRanking'):
			self.invokeFactory('AuthorsRanking',id='AMRanking')
			obj = getattr(self.aq_inner.aq_explicit, 'AMRanking')
			obj.setTitle('AMRanking')
			obj.setMaster(True)

#		if not hasattr(self.aq_inner.aq_explicit, 'AMRanking'):
#			self.invokeFactory('AMRanking',id='AMRanking')
#			obj = getattr(self.aq_inner.aq_explicit, 'AMRanking')
#			obj.setTitle('AMRanking')



	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'updateAllRank')
	def updateAllRank(self):
		"""update all ranking in the content"""
		result = []		
		for name in self.content:
			obj = getattr(self.aq_inner.aq_explicit, name)
			result.append(obj.update())
		return result
		


registerType(RingRanking,PROJECTNAME)								
								
