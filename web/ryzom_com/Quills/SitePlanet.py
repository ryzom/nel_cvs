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


from AccessControl import ClassSecurityInfo
from Products.Archetypes.public import *
from Products.Archetypes import Field, Widget
from Products.Archetypes.Marshall import PrimaryFieldMarshaller
from Products.CMFCore import CMFCorePermissions
from Products.CMFCore.utils import getToolByName
from config import PROJECTNAME

SitePlanetSchema = BaseSchema + Schema((
    IntegerField('entriesPerPage',
        default = 20,
        widget=IntegerWidget(label='Entries Per Page',
            description="Select the number of weblog entries you would like \
                    to display on the front page and any other batched pages.",
            ),
        schemata='configuration'
        ),
    LinesField('includedWeblogs',
        format='checkbox',
        widget=MultiSelectionWidget(label='Blogs to include',
            ),
        vocabulary='getAvailableBlogsVocabulary',
        ),
    ),

    marshall=PrimaryFieldMarshaller(),
    )

SitePlanetSchema['allowDiscussion'].default=0

class SitePlanet(BaseContent):
    """Aggregate all blogs on the site"""

    security = ClassSecurityInfo()
    schema = SitePlanetSchema
    archetype_name = "Site Wide Planet"
    actions = (
        { 'id': 'view',
          'name': 'View',
          'action': 'siteplanet_view',
          'permissions': (CMFCorePermissions.View,) },
        )

    def getAvailableBlogsVocabulary(self):
        """Return a list of all published blogs on the site"""

        catalog = getToolByName(self, 'portal_catalog')
        weblogs = DisplayList()
        for blog in catalog(meta_type='Weblog',review_state='published'):
            weblogs.add(blog,blog.title)

        return weblogs


registerType(SitePlanet, PROJECTNAME)
