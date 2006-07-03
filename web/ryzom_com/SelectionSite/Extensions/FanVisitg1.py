#!/var/Python243/bin/python

import sys
import time
import os
import os.path
import shutil

import logging



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
	
	
	site        = line[index1:index2]
        
        imageIndex  = site.rfind('/')
        
        image       = site[imageIndex+1:]
        
        tempTuple   = (ipAddress, dateString, image)
        
        return tempTuple
    
        
  
    

class FanVisitManager (object):
    def __init__(self):
        self.date = None
        self.currentDateString = None
        
        self.siteDict = { }
        
        
        
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
         
            yield "Fan-%s : %d ips differentes" % (siteName, count)


class SiteDataBase(object):
    def __init__(self, fileName):
        self.fileName = fileName
        
        self.siteDict = { }  
        
    
      
    def add(self, siteName, accessDate, accessNumber):
        
    	
        try:
            
            dateNumberList = self.siteDict[siteName]
	    dateNumberList.append( (accessDate, 0) )
	
            
#            dateNumberList.append( (accessDate, accessNumber) )
 
           
        except KeyError:
            
#           self.siteDict[siteName] = [ (accessDate, accessNumber) ]
            self.siteDict[siteName] = [ (accessDate, 0) ]
	    
	    
	    
    def update(self, siteName, accessDate, accessNumber):
    
    	print siteName
	print self.siteDict.keys()
	
    	if siteName in self.siteDict.keys():
		listValue = self.siteDict[siteName]
		print "listValue", listValue
		
		i= 0
		while (i < len(listValue)):
			print "valu", listValue[0][0]
			if accessDate == listValue[i][0]:
			        print listValue[i][1]
				print accessNumber
				listValue[i] = (listValue[i][0],accessNumber)
				print self.siteDict
			
			i += 1
			
	else: 
		print "sssssssssitedic", self.siteDict
		
		
		
        
       
	    
    def save(self):
        
        global separator
        
        
        siteList = self.siteDict.keys()
	print "siteList", siteList
	print "siteDict", self.siteDict
        
        for site in siteList:
            
            dateNumberList = self.siteDict[site]
	    print "dateNumberListe", dateNumberList
            
            	
            
            for dateNumberTuple in dateNumberList:
		print "dateNumberTuple", dateNumberTuple
                
    	print "siteDict", self.siteDict
    
    
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
        
    
    

def FanVisit(fileName , currentDate):
    
    
    
    logging.basicConfig()
    
    
    siteDataBase = SiteDataBase('siteDataBase.txt')
    

    
    
    fileReader = FileReader()
    fileReader.read(fileName, currentDate)
    
    
    lineInfoExtractor = LineInfoExtractor()
    fanVisitManager   = FanVisitManager()
    
    for line in fileReader.getLine():
	
        result = fanVisitManager.add(lineInfoExtractor.extractInfo(line))
        
        if result == False:
            logging.debug("invalid date in line : %s", line)
	    
	    
    visit = []    
    for line in fanVisitManager.getLine():
	visit.append(line)
	
    return visit                
       	        
"""            
    for dateNumberTuple in fanVisitManager.getLine():
        siteDataBase.add(dateNumberTuple[0], currentDate, dateNumberTuple[1])
	
    for dateNumberTuple in fanVisitManager.getLine():
        siteDataBase.update(dateNumberTuple[0], currentDate, dateNumberTuple[1])

    siteDataBase.save()
"""

    
      
