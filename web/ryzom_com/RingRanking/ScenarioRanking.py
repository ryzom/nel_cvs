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

ScenarioRankingSchema=BaseSchema.copy()+ Schema((
	BooleanField('master',
		default = False,
		widget=BooleanWidget(
			description="Select for Masterless Ranking **not use for the moment**"
		),
	),
))

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


	## {rang : [info sur le scenario]}
	Ranking={}
	security.declareProtected(CMFCorePermissions.View, 'getRanking')
	def getRanking(self):
		"""return the ranking's list"""
		return self.Ranking
	
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setRanking')
	def setRanking(self,d):
		"""set the ranking's list"""
		self.Ranking = d

	## stocker le rÃ©sultat de la requete SQL
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'update')
	def update(self):
		"""update Ranking"""
		ranking_by='rrp_scored'
		if self.getMaster():
			master = 'am_dm'
		else:
			master = 'am_autonomous'
				
		## SQL Request
		try:
			request = self.zsql.SQL_ScenarioRanking(ranking_by=ranking_by,master=master)
		except:
			return 'ScenarioRanking Update Failed'

		## Format Result of the request
		formatted_request=self.FormatRequest(request)
		## store Result formatted
		#self.updateRanking(formatted_request)
		self.setRanking(formatted_request)
		return 'ScenarioRanking Update Success'

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'FormatRequest')
	def FormatRequest(self,request):
		"""retourne un dictionnaire avec le rang comme clef"""
		result = {}
		rank = 0

		for row in request:
			rank+=1
			average_time = 'no stats'
			try:
				average_time = self.zsql.SQL_AverageScenarioTime(scenario_id=row[8])
			except:
				average_time = 'no stats'
			info = {'rank':rank,
				'title':row[0],
				'description':row[1],
				'author':row[2],
				'score':row[3],
				'anim_mode':row[4],
				'language':row[5],
				'orientation':row[6],
				'level':row[7],
				'average_time':average_time,
				}
			result.update({rank:info})
		return result

#	#do nothing for the moment
#	security.declareProtected(CMFCorePermissions.View, 'sortedRanking')
#	def sortedRanking(self,ranking_by=none):
#		"""return a sorted ranking tab"""
#		ranking = self.getRanking()
#		newRanking = ranking		
#		return newRanking

registerType(ScenarioRanking,PROJECTNAME)
