#
# Authors: Brian Skahan <bskahan@etria.com>
#          Tom von Schwerdtner <tvon@etria.com>
#
# Copyright 2004-2005, Etria, LLP
#
# This file is part of Quills
#
# Quills is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Quills is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Quills; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###############################################################################

from Products.Archetypes.public import *
from Products.Archetypes.Marshall import PrimaryFieldMarshaller
from Products.CMFCore import CMFCorePermissions
from config import PROJECTNAME

WeblogDraftsSchema = BaseFolderSchema

class WeblogDrafts(BaseFolder):
    """Weblog Drafts folder"""
    schema = WeblogDraftsSchema
    archetype_name = "Weblog Drafts"
    meta_type = 'WeblogDrafts'
    global_allow = 0
    filter_content_types = 1
    allowed_content_types = ('WeblogEntry',)

    base_view = 'weblogdrafts_view'
    immediate_view = 'weblogdrafts_view'
    actions = (
        { 'id': 'view',
          'name': 'View',
          'action': 'string:${folder_url}/weblogdrafts_view',
          'permissions': (CMFCorePermissions.View,) 
       },{
          'id': 'edit',
          'visible' : 0
         },
       )

    def getDrafts(self):
        return self.getFolderListingFolderContents(contentFilter={
                                               'portal_type':'WeblogEntry'},)

registerType(WeblogDrafts, PROJECTNAME)
