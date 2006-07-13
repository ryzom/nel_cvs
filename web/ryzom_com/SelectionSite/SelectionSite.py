try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *
from Products.CMFCore import CMFCorePermissions
from config import PROJECTNAME

import time

SelectionSchema = BaseSchema.copy() + Schema((   
	TextField('description',
		searchable=1,
		required=0,
		widget=TextAreaWidget(label='Summary Description',
			description='description of the site',
		),              
	),
	TextField('text',
		searchable=1,	               
		widget=EpozWidget(label='Item Description',
			description='description of the site',
		),
		required=1,
		default_output_type='text/html',
		allowable_content_types=('text/plain','text/structured','text/html'),
	),
	StringField("url",
		searchable = 0,
		required = 1,
		widget=StringWidget(label="URL/website",
 			label_msgid="label_url",
			description="A link to the official website",
			description_msgid="desc_url",
			validator="isURL",
		),
	),	      
	ImageField('logo',
		required=1,
		default_output_type='image/jpeg',
		allowable_content_types=('image/*',),          
		widget=ImageWidget(description='upload a logo of the site'),
	),
	LinesField('category',
		searchable=1,
		required=0,
		vocabulary=['News','Tutorial'],            
		widget=SelectionWidget(description='select your category?'),
	),
	LinesField('visit',		
		widget=LinesWidget(description=''),
	),
	LinesField('month',		
		widget=LinesWidget(description=''),
	),
	IntegerField('monthsum',
		default = 0,
		widget=IntegerWidget(description=''),
	),
	IntegerField('allsum',
		default = 0,
		widget=IntegerWidget(description=''),
	),
	),
)
  
    

class SelectionSite(BaseContent):
	"""Selection Site"""
	schema = SelectionSchema
	archetype_name = "SelectionSite"
	meta_type = 'SelectionSite'
	default_view  = 'SelectionSite_view'
	immediate_view = 'SelectionSite_view'

	actions = (
	{
		'id'          : 'view',
		'name'        : 'View',
 		'action'      : 'string:${object_url}/SelectionSite_view',
 		'permissions' : (CMFCorePermissions.View,)
	},
#	{
#		'id'          : 'edit',
#		'name'        : 'Edit',
#		'action'      : 'string:${object_url}/SelectionSite_edit',
#		'permissions' : (CMFCorePermissions.ModifyPortalContent,),
#	},
	)

	def setTitle(self, value, **kwargs):
		self.getField('title').set(self, value, **kwargs)
		if value:
			try:
				self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())
			except:
				pass #try to do better than this


	#create dictionnary with Visit
	def VisitDico(self):
		visit = self.getVisit()
		dico_visit = {}
		for line in visit:
			t=line.split(':')
			key=t[0]
			value=t[1]
			dico_visit.update({key:value})
		return dico_visit


	def addVisit(self,dico):
		current_visit = self.VisitDico()
		current_visit.update(dico)
		listkey = current_visit.keys()
		listkey = self.sortByDate(listkeys)
		updated_visit=[]
		allsum = 0
		for key in listkey:
			updated_visit.append(str(key)+':'+str(current_visit[key]))
			allsum += int(current_visit[key])
		self.setAllsum(allsum)
		self.setVisit(updated_visit)		

	def VisitThisMonth(self):
		visit = self.VisitDico()
		currentMonth = time.strftime('%b/%Y', time.localtime())
		ThisMonth = []
		monthsum = 0
		keys = visit.keys()
		keys = self.sortByDate(keys)
		for key in keys:			
			if key.split('/',1)[1] == currentMonth:
				ThisMonth.append(str(key)+':'+str(visit[key]))
				monthsum += int(visit[key])
		self.setMonthsum(monthsum)
		self.setMonth(ThisMonth)

	def format(self,date):
		monthDic = {'Jan':1, 'Feb':2, 'Mar':3, 'Apr':4, 'May':5, 'Jun':6,
		            'Jul':7, 'Aug':8, 'Sep':9, 'Oct':10,'Nov':11, 'Dec':12 }
		date = date.split('/')
		#date = date.join(',')
		month = date[1]
		inter = date[0]
		if month in monthDic.keys():
			month = monthDic[month]
		date[1] = month
		date[0] = date[2]
		date[2] = inter
		return date

	def deformat(self,date):
		monthDic = { 1: 'Jan', 2: 'Feb', 3: 'Mar', 4: 'Apr', 5: 'May', 6: 'Jun',
		             7: 'Jul', 8: 'Aug', 9: 'Sep', 10: 'Oct', 11: 'Nov', 12: 'Dec'}
		month = date[1]	
		if month in monthDic.keys():
			month = monthDic[month]
		return str(date[2])+'/'+str(month)+'/'+str(date[0])

	def sortByDate(self,date):
		dateList = []
		for date in date:
			date = format(date)
			dateList.append(date)
		dateListSorted=[]
		dateList.sort()
		for date in dateList:
			dateListSorted.append(deformat(date))
		return dateListSorted

#	def sortVisit(self,dico):
#		listkey = dico.keys()
#		newlistkey=[]
#		for key in listkey:
#			date = key.split('/')
#			day = date[1]
#			month = date[2]
#			year = date[3]
#			newkey = ''.join([year,month,day],'/')
#			newlistkey.append(newkey)
#		newlistkey.sort()
#		return dico
		


registerType(SelectionSite, PROJECTNAME)
