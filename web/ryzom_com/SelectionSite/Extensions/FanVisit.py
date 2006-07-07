#!/var/Python243/bin/python

import sys
import time
import os
import os.path
import shutil

import logging
#import FilterSite


loggingLevel = logging.DEBUG  # { NOTSET, DEBUG, INFO, WARNING, ERROR, CRITICAL }




class FileReader (object):
    def __init__(self):
        self.lineList = []
        self.currentDate = None        
    
    def read(self, fileName, currentDate):        
        self.currentDate = currentDate        
        self.lineList   = [x for x in open(fileName) if self.checkDate(x)]
    
    def getLine(self):
        for line in self.lineList:
            yield line        
            
    def checkDate(self, line):        
        index1      = line.find('[')
        index2      = line.find(':', index1)        
        dateString  = line[index1+1:index2]        
        if dateString == self.currentDate:
            return True
        else:
            return False
        
        
class LineInfoExtractor (object):
    def __init__(self):
        pass
    
    def extractInfo(self, line):        
        index     = line.find(' - -')
        ipAddress = line[:index]
        index1      = line.find('[', index)
        index2      = line.find(':', index1)        
        dateString  = line[index1+1:index2]        
        index1      = line.find(r'"', index2)
        index2      = line.find(r'"', index1+1)
	site        = line[index1:index2]        
        imageIndex  = site.rfind('/')
        image       = site[imageIndex+1:]
        tempTuple   = (ipAddress, dateString, image)
        return tempTuple

    def listSite(self,line):    
    	index1      = line.find(r'"', index2)
        index2      = line.find(r'"', index1+1)        
        site        = line[index1:index2]
    	listSite = []	
	if site not in listSite:
		listSite.append(self.site)
	return listSite   
    

class FanVisitManager (object):
    def __init__(self):
        self.date = None
        self.currentDateString = None        
        self.siteDict = {}
        
    def add(self, info):        
        ipAddress  = info[0]
        dateString = info[1]
        siteName   = info[2]        
	logging.debug("self.currentDateString = %s" %self.currentDateString)
        logging.debug("ipAddress = %s  dateString = %s  siteName = %s" %(ipAddress, dateString, siteName))
        
        if self.currentDateString == None:            
            logging.debug("first line")
            self.currentDateString = dateString            
        elif self.currentDateString != dateString:            
            logging.debug("line rejected")
            return False        
        
        try:
            ipDict = self.siteDict[siteName]
            logging.debug("site already exist")
            
            try:
                ipfound = ipDict[ipAddress]
                logging.debug("ip already exist for this site (%s)" %siteName)
                
            except KeyError:
                logging.debug("ip does not exist for this site(%s)" %siteName)                
                ipDict[ipAddress] = None                
                logging.debug("ip (%s) added" %ipAddress)
            
        except KeyError:
            logging.debug("site dose not exist")            
            self.siteDict[siteName] = { ipAddress:None }
            logging.debug("site (%s) added  %s" %(siteName, ipAddress))            
            
        return True
    
    
    def getLine(self):        
        keyList = self.siteDict.keys()        
        for siteName in keyList:
            ipDict = self.siteDict[siteName]            
            count = len(ipDict.keys())            
            yield (siteName, count)
	    
            


class SiteDataBase(object):
    def __init__(self):
        self.siteDict = {}

    def addDate(self, listSite ):    
	date = time.strftime('%d/%b/%Y', time.localtime())
#	print listSite	
#	for site in FilterSite.getSite():
#	listSite = ['site1', 'site2', 'site3','site4', 'site5', 'site6', 'site7', 'site8', 'site9']
	for site in listSite:
#		try:
#            		dateNumberList = self.siteDict[site]
#			dateNumberList.append( (date, 0) )
		self.siteDict.update({site:{date:0}})
#		except KeyError:
#			self.siteDict[site] = [ (date, 0) ]
	    
#    def update(self, siteName, accessDate, accessNumber):    
#    	print 'siteName' , siteName
#	print 'site.keys' , self.siteDict.keys()
#	print 'accessdate', accessDate	
#    	if siteName in self.siteDict.keys():
#		listValue = self.siteDict[siteName]
#		print "listValue", listValue		
#		i= 0
#		while (i < len(listValue)):
#			print "value", listValue[0][0]
#			if accessDate == listValue[i][0]:
#			        print listValue[i][1]				
#				listValue[i] = (listValue[i][0],accessNumber)
#				print self.siteDict			
#			i += 1			
#	else: 
#		print "sssssssssitedic", self.siteDict
	    
#    def save(self):
#        siteList = self.siteDict.keys()
#	print "siteList", siteList
#	print "siteDict", self.siteDict
#       for site in siteList:            
#            dateNumberList = self.siteDict[site]
#	    print "dateNumberListe", dateNumberList
#            for dateNumberTuple in dateNumberList:
#		print "dateNumberTuple", dateNumberTuple                
#    	print "savesiteDict", self.siteDict
    
    
    def getAll(self):
        global separator
	self.accessList = []
	i= 2	
    	Index1= line.find[separator]
	siteName = line[0, index1]
 	accessList.append(siteName)	
	for i in len(line):
	 	indexStart = index+str(i)
		indexEnd   = index+str(i+1)  
		indexStart = line.find[',']
		indexEnd   = line.find[',', indexStart]
		NumberAccess= line[indexStart: indexEnd]
  		accessList.append(NumberAccess)  
        
    
    
def FanVisit(fileName , listSite) :
#if __name__ == '__main__':
#    fileName = sys.argv[1]
#    fileName = 'test.txt'
#   listSite = sys.argv[2]
    currentDate = time.strftime('%d/%b/%Y', time.localtime())
#    listSite = ['site1', 'site2', 'site3','site4', 'site5', 'site6', 'site7', 'site8', 'site9']
    
    logging.basicConfig()
    siteDataBase = SiteDataBase()
    fileReader = FileReader()
    fileReader.read(fileName, currentDate)
    lineInfoExtractor = LineInfoExtractor()
    fanVisitManager   = FanVisitManager()
    
    for line in fileReader.getLine():
        result = fanVisitManager.add(lineInfoExtractor.extractInfo(line))
        if result == False:
            logging.debug("invalid date in line : %s", line)
    siteDataBase.addDate(listSite)

    for dateNumberTuple in fanVisitManager.getLine():
#		print 'hello'
#		siteDataBase.update(dateNumberTuple[0], currentDate, dateNumberTuple[1])
		siteDataBase.siteDict.update({dateNumberTuple[0]:{currentDate:dateNumberTuple[1]}})
#    siteDataBase.save()
    return siteDataBase.siteDict

    


    
      
