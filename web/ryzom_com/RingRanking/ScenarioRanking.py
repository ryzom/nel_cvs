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
from RankingTool import fusion

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
	existingLanguage = []

	security.declareProtected(CMFCorePermissions.View, 'getExistingLanguage')
	def getExistingLanguage(self):
		"""return a tuple with different language used for scenario"""
		return self.existingLanguage

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setExistingLanguage')
	def setExistingLanguage(self):
		"""set the list of language used in scenario"""
		langs = []
		try:
			request = self.zsql.SQL_ScenarioExistingLanguage()
		except:
			request = []
		for row in request.dictionaries():
			langs.append(row['language'])
		self.existingLanguage = langs

		
	## {rang : [info sur le scenario]}
	Ranking=[]

	security.declareProtected(CMFCorePermissions.View, 'getRanking')
	def getRanking(self,langs=None):
		"""return the ranking's list, filter by language passed in a string like  "en,fr,de" )"""
		if not langs :
			return self.Ranking
		langs = langs.split(',')
		filter_ranking = []
		ranking = self.Ranking
		for row in ranking:
			if row['language'] in langs:
				filter_ranking.append(row)
		if filter_ranking:
			return filter_ranking
		return "[]"

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setRanking')
	def setRanking(self,d):
		"""set the ranking's list"""
		self.Ranking = d



	RankingFR=[]
	RankingEN=[]
	RankingDE=[]
	RankingOther=[]

	security.declareProtected(CMFCorePermissions.View, 'getRankingLang')
	def getRankingLang(self,lang):
		"""set the ranking's list"""
		lang = str(lang)
		if lang == 'fr':
			return self.RankingFR
		if lang == 'en':
			return self.RankingEN
		if lang == 'de':
			return self.RankingDE
		if lang == 'other':
			return self.RankingOther

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setRankingLang')
	def setRankingLang(self,d,lang):
		"""set the ranking's list"""
		lang = str(lang)
		if lang == 'fr':
			self.RankingFR = d
		if lang == 'en':
			self.RankingEN = d
		if lang == 'de':
			self.RankingDE = d
		if lang == 'other':
			self.RankingOther = d

	security.declareProtected(CMFCorePermissions.View, 'getRankings')
	def getRankings(self,langs=None,limit=10):
		"""get ranking for each language selected and return a sorted by rank tab"""
		## if not lang return international tab
		if not langs :
			return self.Ranking
		
		## else create a tab with all language selected
		langs = langs.split(',')
		limit = int(limit)
		result = []
		for lang in langs:
			if lang and lang != '' and lang != ',':
				tab = self.getRankingLang(str(lang))
				if len(tab) >0:
					result = fusion(result,tab,limit)
		if result:
			return result
		return "[]"


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
		formatted_request=self.FormatRequest(req)
		self.setRanking(formatted_request)
		self.setExistingLanguage()

		## update Ranking FR,EN,DE
		for lang in ['fr','en','de']:
			try:
				request = self.zsql.SQL_ScenarioRankingByLang(ranking_by=ranking_by,master=master,language=lang)
			except:
				return 'ScenarioRanking Update Failed'
			if len(request) > limit:
				req=request.dictionaries()[0:limit]
			else:
				req = request
			formatted_request=self.FormatRequest(req)
			self.setRankingLang(formatted_request,lang)

		##update for other's language
		try:
			request = self.zsql.SQL_ScenarioRankingByOtherLang(ranking_by=ranking_by,master=master)
		except:
			return 'ScenarioRanking Update Failed'
		if len(request) > limit:
			req=request.dictionaries()[0:limit]
		else:
			req = request
		formatted_request=self.FormatRequest(req)
		self.setRankingLang(formatted_request,'other')
		return 'ScenarioRanking Update Success'

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'FormatRequest')
	def FormatRequest(self,request):
		"""retourne un tableau de dictionnaire"""
		result = []
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
			result.append(info)
		return result

#	#do nothing for the moment
#	security.declareProtected(CMFCorePermissions.View, 'sortedRanking')
#	def sortedRanking(self,ranking_by=none):
#		"""return a sorted ranking tab"""
#		ranking = self.getRanking()
#		newRanking = ranking		
#		return newRanking

registerType(ScenarioRanking,PROJECTNAME)
