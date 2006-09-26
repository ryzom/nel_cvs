from Products.ExternalMethod.ExternalMethod import manage_addExternalMethod
from Products.CMFCore.utils import getToolByName

from Products.Archetypes.public import listTypes
from Products.Archetypes.Extensions.utils import installTypes, install_subskin
from Products.RingRanking.config import PROJECTNAME, GLOBALS
from StringIO import StringIO

def install_maintenancetask(self,action='install'):
	mtool = getToolByName(self, 'portal_maintenance', None)
	if not mtool:
		return
	if action in ('install', 'reinstall'):
		task_id = task_title = task = 'runUpdateRingRanking'
		manage_addExternalMethod(mtool.scripts,task_id,task_title,PROJECTNAME+'.MaintenanceToolbox',task)
		try:
			mtool.tasks.manage_addProduct['PloneMaintenance'].addMaintenanceTask(task_id)
		except:
			pass
		obj = getattr(mtool.tasks, task_id)
		obj.scheduled_month = '*'
		obj.scheduled_day_of_month = '*'
		obj.scheduled_hour = '0'
		obj.scheduled_minute = '0'
	        obj.script_name = task_id
	else:
		if task_id in mtool.tasks.objectIds():
			mtool.tasks.manage_delObjects(task_id)
		if task in mtool.scripts.objectIds():
			mtool.scripts.manage_delObjects(task)


def install(self,reinstall=False):

#	if not reinstall:
#		install_maintenancetask(self, 'install')

	out = StringIO()    
	installTypes(self, out, listTypes(PROJECTNAME), PROJECTNAME)
	install_subskin(self, out, GLOBALS)

	#print>>out("Successfully installed %s." % PROJECTNAME)
	return out.getvalue()

#def uninstall(self):
#    install_maintenancetask(self, 'uninstall')
