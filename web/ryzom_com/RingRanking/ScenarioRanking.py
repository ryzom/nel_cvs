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
	BooleanField('masterless',
		default = False,
		widget=BooleanWidget(
			description="Select for Masterless Ranking **not use for the moment**"
		),
	),
#	LinesField('lang',
#		required=True,
#		vocabulary=['en','fr','de'],
#		widget=SelectionWidget(
#			description="Choose a language",
#		),
#	),
))

class ScenarioRanking(BaseContent):
	"""Don't **use** this object"""
	security = ClassSecurityInfo()
	schema = ScenarioRankingSchema
	meta_type = portal_type = 'ScenarioRanking'
	global_allow = 0
	_at_rename_after_creation = True

	actions = (
		{ 'id': 'view',
		'name': 'View',
		'action': 'string:${object_url}/ScenarioRanking_view',
		'permissions': (CMFCorePermissions.View,)
		},
	)

	## existing language
	existingLanguage = ()
	security.declareProtected(CMFCorePermissions.View, 'getExistingLanguage')
	def getExistingLanguage(self):
		"""return a tuple with different language used for scenario"""
		return self.existingLanguage

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setExistingLanguage')
	def setExistingLanguage(self):
		"""set the list of language used in scenario"""
		langs = ()
		try:
			request = self.zsql.SQL_ScenarioExistingLanguage()
		except:
			request = []
		for row in request.dictionaries():
			langs+=(row['language'],)
		self.existingLanguage = langs

		
	## {rang : [info sur le scenario]}
	Ranking={}
	security.declareProtected(CMFCorePermissions.View, 'getRanking')
	def getRanking(self,langs=()):
		"""return the ranking's list, filter by language passed in a tuple like ('en','fr','de')"""
		if not langs :
			return self.Ranking

		filter_ranking = {}
		ranking = self.Ranking
		keys = ranking.keys()
		for key in keys:
			if ranking[key]['language'] in langs:
				filter_ranking.update({key:ranking[key]})
		if filter_ranking:
			return filter_ranking
		return "{}"
	
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setRanking')
	def setRanking(self,d):
		"""set the ranking's list"""
		self.Ranking = d

	## stocker le rÃ©sultat de la requete SQL
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'update')
	def update(self,limit=10):
		"""update Ranking"""
		ranking_by='rrp_scored'
		limit = int(limit)
		req = []
		if self.getMasterless():
			master = 'am_autonomous'
		else:
			master = 'am_dm'				

		## SQL Request
		try:
			request = self.zsql.SQL_ScenarioRanking(ranking_by=ranking_by,master=master)
		except:
			return 'ScenarioRanking Update Failed'

		if len(request) > limit:
			req=request.dictionaries()[0:limit]
		else:
			req = request

		## Format Result of the request
		formatted_request=self.FormatRequest(req)
		## store Result formatted
		self.setRanking(formatted_request)
		##store language
		self.setExistingLanguage()
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
				#this SQL return one row of one column
				average_time = self.zsql.SQL_AverageScenarioTime(scenario_id=row['id'])[0][0]
			except:
				average_time = 'no stats'
			info = {'rank':rank,
				'title':row['title'],
				'description':row['description'],
				'author':row['author'],
				'score':row['rrp_total'],
				'language':row['language'],
				'orientation':row['orientation'],
				'level':row['level'],
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
