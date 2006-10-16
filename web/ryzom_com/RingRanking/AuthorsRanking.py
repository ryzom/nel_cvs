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


	## [{info sur le scenario}]
	Ranking=[]
	security.declareProtected(CMFCorePermissions.View, 'getRanking')
	def getRanking(self):
		"""return the ranking's list"""
		if self.Ranking:
			return self.Ranking

		return "[]"
	
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setRanking')
	def setRanking(self,d):
		"""set the ranking's list"""
		self.Ranking = d

	RankingLeanon=[]
	RankingArispotle=[]
	RankingAniro=[]
	RankingCho=[]
	RankingToo=[]

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'getRankingLang')
	def getRankingServer(self,server):
		"""set the ranking's list"""
		server = str(server)
		if server == 'Leanon':
			return self.RankingLeanon
		if server == 'Arispotle':
			return self.RankingArispotle
		if server == 'Aniro':
			return self.RankingAniro
		if server == 'Cho':
			return self.RankingCho
		if server == 'Too':
			return self.RankingToo
		return []
 

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setRankingLang')
	def setRankingServer(self,d,server):
		"""set the ranking's list"""
		server = str(server)
		if server == 'Leanon':
			self.RankingLeanon = d
		if server == 'Arispotle':
			self.RankingArispotle = d
		if server == 'Aniro':
			self.RankingAniro = d
		if server == 'Cho':
			self.RankingCho = d
		if server == 'Too':
			self.RankingToo = d

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'getRankings')
	def getRankings(self,servers=None,limit=10):
		"""get ranking for each language selected and return a sorted by rank tab"""
		## if not lang return international tab
		if not servers :
			return self.Ranking
		
		## else create a tab with all language selected
		servers = servers.split(',')
		limit = int(limit)
		result = []
		for server in servers:
			if server and server != '' and server != ',':
				tab = self.getRankingServer(str(server))
				if len(tab) >0:
					result = fusion(result,tab,limit)
		if result:
			return result
		return "[]"




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
		text=""
		if self.getAM():
			ranking_by = 'rrp_am'
		else:
			ranking_by = 'rrp_author'		
		## SQL Request		
		try:
			request = self.zsql.SQL_AuthorsRanking(ranking_by=ranking_by)
		except:
			return 'Author Ranking Update Failed'
		text+="Author Ranking (all server) update success\n"
		
		if len(request) > limit:
			req = request.dictionaries()[0:limit]
		else:
			req = request
		
		## Format Result of the request
		formatted_request=self.FormatRequest(req)
		## store Result formatted
		self.setRanking(formatted_request)
		list_server = ["Leanon","Aniro","Arispotle","Cho"]
		list_server = ["Too"]
		for server_name in list_server:
			#try:
			#	request = self.zsql.SQL_AuthorsRankingByServer(ranking_by=ranking_by,server_name=server_name)
			#except:
			#	return text+" Ranking "+str(server_name)+" Update Failed"
			request = self.zsql.SQL_AuthorsRankingByServer(ranking_by=ranking_by,server_name=server_name)
			text+="general update success ("+str(server_name)+")\n"
			if len(request) > limit:
				req = request.dictionaries()[0:limit]
			else:
				req = request
			formatted_request=self.FormatRequest(req)
			self.setRankingServer(formatted_request,server_name)

		
		return text

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'FormatRequest')
	def FormatRequest(self,request):
		"""retourne un tableau de dictionnaire"""
		result = []
		rank = 0
		for row in request:
			rank+=1
			guild = ''
			pioneer = 0
			mainland = ''
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

			#get mainland
			try:
				mainland = self.zsql.SQL_GetMainland(mainland_id=row['mainland_id'])[0][0]
			except:
				mainland = ''

			#create information
			if self.isAdventureMaster():
				info = {'rank':rank,
					'name':row['char_name'],
					'guild':guild,
					'pioneer':pioneer,
					'score':row['rrp_am'],
					'language':row['lang'],
					'mainland':mainland,
					}
			else:
				info = {'rank':rank,
					'name':row['char_name'],
					'guild':guild,
					'pioneer':pioneer,
					'score':row['rrp_author'],
					'language':row['lang'],
					'mainland':mainland,
					}
			#update tab
			result.append(info)
		return result


#	security.declareProtected(CMFCorePermissions.View, 'sortedRanking')
#	def sortedRanking(ranking_by=none):
#		"""return a sorted ranking tab"""
#		ranking = self.getRanking()
#		return ranking

		

registerType(AuthorsRanking,PROJECTNAME)
