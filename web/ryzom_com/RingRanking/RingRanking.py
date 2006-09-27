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

RingRankingSchema=BaseFolderSchema.copy()+ Schema((
	LinesField('lang',
		required=True,
		vocabulary=['en','fr','de'],
		widget=SelectionWidget(
			description="Choose a language",
			description_msgid="RingRanking_schema_lang",
			i18n_domain="RingRanking",
		),
	),
	TextField('description',
		widget=TextAreaWidget(
			label='Summary Description',
			label_msgid="RingRanking_schema_label_desc",
			description='description',
			description_msgid="RingRanking_schema_desc",
			i18n_domain="RingRanking",
		),
	),
))
 
 
   
class RingRanking(BaseFolder):
	"""Add an Ring Ranking Folder"""

	security = ClassSecurityInfo()
	schema = RingRankingSchema
	archetype_name = meta_type = 'RingRanking'
	allowed_content_types = ['AuthorsRanking',				 
				 'ScenarioRanking',]
	_at_rename_after_creation = True

	#name (title & id) of the content created
	content = ['AMRanking',
		   'AuthorsRanking',
		   'ScenarioMasterlessRanking',
		   'ScenarioMasteredRanking',]


	actions = (
		{ 'id': 'view',
		'name': 'view',
		'action': 'string:${object_url}/RingRanking_view',
		'permissions': (CMFCorePermissions.View,)
		},
	)

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'initializeArchetype')
	def initializeArchetype(self, **kwargs):
		"""use automatically at creation"""
		BaseFolder.initializeArchetype(self, **kwargs)

		if not hasattr(self.aq_inner.aq_explicit, 'ScenarioMasteredRanking'):
			self.invokeFactory('ScenarioRanking',id='ScenarioMasteredRanking')
			obj = getattr(self.aq_inner.aq_explicit, 'ScenarioMasteredRanking')
			obj.setTitle('ScenarioMasteredRanking')
			obj.setMasterless(False)

		if not hasattr(self.aq_inner.aq_explicit, 'ScenarioMasterlessRanking'):
			self.invokeFactory('ScenarioRanking',id='ScenarioMasterlessRanking')
			obj = getattr(self.aq_inner.aq_explicit, 'ScenarioMasterlessRanking')
			obj.setTitle('ScenarioMasterlessRanking')
			obj.setMasterless(True)

		if not hasattr(self.aq_inner.aq_explicit, 'AuthorsRanking'):
			self.invokeFactory('AuthorsRanking',id='AuthorsRanking')
			obj = getattr(self.aq_inner.aq_explicit, 'AuthorsRanking')
			obj.setTitle('AuthorsRanking')
			obj.setAM(False)

		if not hasattr(self.aq_inner.aq_explicit, 'AMRanking'):
			self.invokeFactory('AuthorsRanking',id='AMRanking')
			obj = getattr(self.aq_inner.aq_explicit, 'AMRanking')
			obj.setTitle('AMRanking')
			obj.setAM(True)


	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setLang')
	def setLang(self, value, **kwargs):
		"""set language use for content's sql request"""
		self.getField('lang').set(self, value, **kwargs)
		for name in self.content:
			obj = getattr(self.aq_inner.aq_explicit, name)
			obj.setLang(value)


	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'updateAllRank')
	def updateAllRank(self):
		"""update all ranking in the content"""
		result = []		
		for name in self.content:
			obj = getattr(self.aq_inner.aq_explicit, name)
			result.append(obj.update())
		return result
		


registerType(RingRanking,PROJECTNAME)								
								
