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
##############################################################################

from Globals import package_home
from Products.Archetypes.public import *
from Products.CMFCore import utils
from Products.CMFCore.DirectoryView import registerDirectory
import os, os.path

#__version__ = open(os.path.join(__path__[0], 'version.txt')).read().strip()

from config import *
from permissions import initialize as initialize_permissions
import QuillsTool

tools = (QuillsTool.QuillsTool,)

def initialize(context):

    import Weblog
    import WeblogEntry
    import WeblogArchive
    import WeblogDrafts
    import WeblogTopic
#    import SitePlanet

    content_types, constructors, ftis = process_types(
        listTypes(PROJECTNAME),
        PROJECTNAME)

    utils.ToolInit('Quills Tool', tools=tools,
            product_name=PROJECTNAME, icon='tool.gif',
            ).initialize(context)

    permissions = initialize_permissions()
    allTypes = zip(content_types, constructors)
    for atype, constructor in allTypes:
        kind = "%s: %s" % (config.PROJECTNAME, atype.archetype_name)
        utils.ContentInit(
            kind,
            content_types      = (atype,),
            permission         = permissions[atype.portal_type],
            extra_constructors = (constructor,),
            fti                = ftis,
            ).initialize(context)

registerDirectory(SKINS_DIR, GLOBALS)
