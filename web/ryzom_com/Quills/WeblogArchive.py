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

import DateTime

WeblogArchiveSchema = BaseFolderSchema + Schema((
    DateTimeField('date',
        required=1,
        searchable=1,
        accessor='ArchiveDate',
    ),
    StringField('archive_type',
        required=1,
        searchable=0,
        accessor='ArchiveType',
    ),
    ),
    marshall=PrimaryFieldMarshaller(),
    )

class WeblogArchive(BaseFolder):
    """Weblog Archive: There are 4 basic styles, root, year, month, day"""

    schema = WeblogArchiveSchema
    archetype_name = "Weblog Archive"
    meta_type = 'WeblogArchive'
    global_allow = 0
    filter_content_types = 1
    allowed_content_types = ('WeblogEntry',)
    default_view = 'weblogarchive_view'
    immediate_view = 'weblogarchive_view'
    actions = (
            {
            'id': 'view',
            'name': 'Archive',
            'action': 'string:${folder_url}/weblogarchive_view',
            'permissions': (CMFCorePermissions.View,)
            },
            {
            'id': 'edit',
            'visible' : 0
            },
        )

    def __init__(self, oid, **kwargs):
        """Setup the archive folderish object, turn off discussion"""
        BaseFolder.__init__(self, oid, **kwargs)
        self.schema['allowDiscussion'].default=0

    def Title(self):
        """Set the Title attribute to fit yyyy/mm/dd syntax"""

        t = self.ArchiveType()

        if t == "root":
            # TODO: Localize this (?)
            return "Archive"

        date = self.ArchiveDate()

        if date:
            year = str(date.year())
            month = str(date.Month())
            day = str(date.dd())

            if t == "year":
                return year
            elif t == "month":
                return month
            elif t == "day":
                return day

        # Fallback
        return self.getId()

    def getArchiveListing(self):
        """Get a listing of all sub-archives"""

        #This used to be done directly in tal, it wasn't pretty
        return self.getFolderListingFolderContents(
                                contentFilter={'portal_type': 'WeblogArchive','sort_on' : 'id'},
                                suppressHiddenFiles=1)

    def getArchiveEntryListing(self):
        """Get a listing of all entries in this archive node"""

        return self.getFolderListingFolderContents(
                                contentFilter={'portal_type': 'WeblogEntry','sort_on' : 'sortable_title'},
                                suppressHiddenFiles=1)

    def getLazyEntries(self):
        """Return a list of lazy_maps for all entries under this archive"""

        path = '/'.join(self.getPhysicalPath())
        results = self.portal_catalog(
                meta_type=['WeblogEntry',],
                path={'query':path, 'level': 0},
                sort_on = 'effective',
                sort_order = 'reverse',
                review_state = 'published')

        return results

    def getNumberOfEntries(self):
        """simple counter method for lazy entries"""

        return len(self.getLazyEntries())

    def getEntries(self):
        """Return a list of objects for all entries under this archive"""

        maps = self.getLazyEntries()
        results = []
        for entry in maps:
            results.append(r.getObject())
        return results

    def getRootArchive(self):
        """Return the 'root' archive for this archive"""

        return self._getRootArchive(self)

    def _getRootArchive(self, a):
        """Recurse down to the 'root' archive object"""

        # Sanity check
        if a.meta_type != 'WeblogArchive':
            return None

        # If this is the root, return it, else recurse
        if a.ArchiveType() == 'root':
            return a
        else:
            return self._getRootArchive(a.aq_parent)

    def createPath(self, s):
        date = DateTime.DateTime(s)
        year = str(date.year())
        month = str(date.mm())
        day = str(date.dd())

        if self.archive_type != 'root':
            print "You can only build paths in a root archive"
            # FIXME: raise error (or find the root archive and construct the
            # path from there)
            # FIXME: There is some issue where if the day and the month are the
            # same number (eg 2004/08/08) then the archive object returned is
            # the month and not the day.  I think there is something going on
            # with running getattr(ojb-with-id-08, '08').

        path = self
        if not hasattr(path.aq_inner.aq_explicit, year):
            self.portal_types.constructContent('WeblogArchive',
                    path, year, title=year, archive_type='year', date = s) 

        path = getattr(path.aq_inner.aq_explicit, year)
        if not hasattr(path.aq_inner.aq_explicit, month):
            self.portal_types.constructContent('WeblogArchive',
                    path, month, title=month, archive_type='month', date = s)

        path = getattr(path.aq_inner.aq_explicit, month)
        if not hasattr(path.aq_inner.aq_explicit, day):
            self.portal_types.constructContent('WeblogArchive',
                    path, day, title=day, archive_type='day', date = s)

        path = getattr(path.aq_inner.aq_explicit, day)
        return path

registerType(WeblogArchive, PROJECTNAME)
