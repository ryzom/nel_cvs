from Products.CMFCore.CMFCorePermissions import setDefaultRoles
import Products.Archetypes.public as atapi
import config

def initialize():
    permissions = {}
    types = atapi.listTypes(config.PROJECTNAME)
    for atype in  types:
        permission = "%s: Add %s" % (config.PROJECTNAME, atype['portal_type'])
        permissions[atype['portal_type']] = permission
        setDefaultRoles(permission, ('Manager','Owner'))

    ManageTeamMembership = 'Manage team memberships'
    setDefaultRoles(ManageTeamMembership, ('Manager',))

    return permissions
