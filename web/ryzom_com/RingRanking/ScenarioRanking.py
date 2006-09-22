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

ScenarioRankingSchema=BaseSchema.copy()+ Schema(())

class ScenarioRanking(BaseContent):
	"""Don't **use** this object"""
	security = ClassSecurityInfo()
	schema = ScenarioRankingSchema
	meta_type = portal_type = 'ScenarioRanking'
	global_allow = 0
	_at_rename_after_creation = True

#	actions = (
#		{ 'id': 'view',
#		'name': 'View',
#		'action': 'string:${object_url}/ScenarioRanking_view',
#		'permissions': (CMFCorePermissions.View,)
#		},
#	)


#	au cas ou on fonctionnerais uniquement en attaquant la bdd, il faudrais penser a gere les types complexes	
#	## nom du type : colonne SQL
#	ranking_simple={'Rank':'rrp_total',
#			'Name of scenario':'title',
#			'Name of author':'author',
#			'Type of scenario':'orientation',
#			'Mastered / masterless':'anim_mode',
#			'Language':'language',
#			}
#	## nom du type : nom de la requete SQL
#	ranking_complex={'Guild':'SQL_scenarioGuild'}


	## {rang : [info sur le scenario]}
	Ranking={}
	security.declareProtected(CMFCorePermissions.View, 'getRanking')
	def getRanking(self):
		"""return the ranking's list"""
		return self.Ranking
	
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'updateRanking')
	def updateRanking(self,d):
		"""update the ranking's list"""
		self.Ranking.update(d)

	## stocker le rÃ©sultat de la requete SQL
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'update')
	def update(self):
		"""update Ranking"""		
		## SQL Request
		try:
			request = self.SQL_ScenarioRanking(ranking_by='rrp_scored')
		except:
			return 'ScenarioRanking Update Failed'
		## Format Result of the request
		formatted_request=self.FormatRequest(request)
		## store Result formatted
		self.updateRanking(formatted_request)		
		return 'ScenarioRanking Update Success'

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'FormatRequest')
	def FormatRequest(request):
		"""retourne un dictionnaire avec le rang comme clef"""
		result = {}
		rank = 0		
		for row in request:
			rank+=1
			info = {'rank':rank,
				'title':row[0],
				'description':row[1],
				'author':row[2],
				'score':row[3],
				'anim_mode':row[4],
				'language':row[5],
				'orientation':row[6],
				'level':row[7],
				}
			result.update({rank:info})
		return result

	security.declareProtected(CMFCorePermissions.View, 'sortedRanking')
	def sortedRanking(ranking_by=none):
		"""return a sorted ranking tab"""
		ranking = self.getRanking()
		return ranking

	
		



registerType(ScenarioRanking,PROJECTNAME)
