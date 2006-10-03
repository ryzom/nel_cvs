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

AuthorsRankingSchema=BaseSchema.copy()+ Schema((
	BooleanField('AM',
		default = False,
		widget=BooleanWidget(
			description="Select for Animator Ranking **not use for the moment**"
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

class AuthorsRanking(BaseContent):
	"""Don't **use** this object"""
	security = ClassSecurityInfo()
	schema = AuthorsRankingSchema
	meta_type = portal_type = 'AuthorsRanking'
	global_allow = 0
	_at_rename_after_creation = True

	actions = (
		{ 'id': 'view',
		'name': 'View',
		'action': 'string:${object_url}/AuthorRanking_view',
		'permissions': (CMFCorePermissions.View,)
		},
	)


	## {rang : [info sur le scenario]}
	Ranking={}
	security.declareProtected(CMFCorePermissions.View, 'getRanking')
	def getRanking(self,langs=()):
		"""return the ranking's list, filter by language passed in a tuple like ('lang_en','lang_fr','lang_de')"""
		if not langs :
			return self.Ranking

		filter_ranking = {}
		ranking = self.Ranking
		keys = ranking.keys()
		for key in keys:
			if ranking[key]['language'] in langs:
				filter_ranking.update({key:ranking[key]})
		return filter_ranking


	
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setRanking')
	def setRanking(self,d):
		"""set the ranking's list"""
		self.Ranking = d

	security.declareProtected(CMFCorePermissions.View, 'isAdventureMaster')
	def isAdventureMaster(self):
		"""return if the rank is for Adventure Master"""
		return self.getAM()

	## stocker le rÃ©sultat de la requete SQL
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'update')
	def update(self,limit=10):
		"""update Ranking"""
		limit=int(limit)
#		lng=self.getLang()
#		lang = 'lang_en'
#		if 'fr' in lng:
#			lang = 'lang_fr'
#		elif 'de' in lng:
#			lang = 'lang_de'

		if self.getAM():
			ranking_by = 'rrp_am'
		else:
			ranking_by = 'rrp_author'		
		## SQL Request		
		try:
			request = self.zsql.SQL_AuthorsRanking(ranking_by=ranking_by)
		except:
			return 'Ranking Update Failed'
		
		if len(request) > limit:
			req = request.dictionaries()[0:limit]
		else:
			req = request
		
		## Format Result of the request
		formatted_request=self.FormatRequest(req)
		## store Result formatted
		self.setRanking(formatted_request)
		return 'AuthorsRanking Update Success'

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'FormatRequest')
	def FormatRequest(self,request):
		"""retourne un dictionnaire avec le rang comme clef"""
		result = {}
		rank = 0
		for row in request:
			rank+=1
			guild = ''
			pioneer = 0
			#get the guild name
			try:
				#this SQL return one row of one column
				guild = self.zsql.SQL_GuildName(guild_id=row['guild_id'])[0][0]
			except:
				guild = ''

			#get if characters's users is pioneer			
			try:
				#this SQL return one row of one column
				pioneer = self.zsql.SQL_GetPrivileges(user_id=row['user_id'])[0][0]			
			except:
				pioneer = ''
			if 'PIONEER' in pioneer:
				pioneer = 'Pioneer'
			
			#create information
			info = {'rank':rank,
				'name':row['char_name'],
				'guild':guild,
				'pioneer':pioneer,
				'score_am':row['rrp_am'],
				'score_author':row['rrp_author'],
				'language':row['lang'],
				}
			#update dictionnarie
			result.update({rank:info})
		return result


#	security.declareProtected(CMFCorePermissions.View, 'sortedRanking')
#	def sortedRanking(ranking_by=none):
#		"""return a sorted ranking tab"""
#		ranking = self.getRanking()
#		return ranking

		

registerType(AuthorsRanking,PROJECTNAME)
