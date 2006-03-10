#
# Authors: Tom von Schwerdtner <tvon@etria.com>
#          Brian Skahan <bskahan@etria.com>
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

from Products.CMFCore.utils import UniqueObject
from Products.CMFCore.utils import getToolByName
from AccessControl import ClassSecurityInfo
from OFS.SimpleItem import SimpleItem
from Globals import InitializeClass
import re
import urllib

class QuillsTool (UniqueObject, SimpleItem):
    """A set of utility methods for Quills"""

    id = 'quills_tool'
    meta_type= 'Quills Tool'
    default_topic_image='default-topic-icon.jpg',
    security = ClassSecurityInfo()
    plone_tool = 1

    security.declarePublic('getByUID')
    def getByUID(self, uid):
        """Shortcut method for the [Blogger,MetaWeblog]API code"""
        uid_catalog = getToolByName(self, 'uid_catalog')
        lazy_cat = uid_catalog(UID=uid)
        o = lazy_cat[0].getObject()
        return o

    security.declarePublic('getWeblogEntriesDates')
    def getWeblogEntriesDates(self, entries):
        """Return a sorted list of dates for dict of entries"""
        days = entries.keys()
        days.sort()
        days.reverse()
        return days

    security.declarePublic('getParentWeblog')
    def getParentWeblog(self, item):
        """Return a Weblog instance above the current object."""
        parent = item.getParentNode()

        if item.meta_type == 'Weblog':
            return item
        elif parent.meta_type == 'Weblog':
            return parent
        elif parent.meta_type == 'Plone Site':
            return None
        else:
            return self.getParentWeblog(parent)

        return None

    security.declarePublic('findRPCAuth')
    def findRPCAuth(self, parent):
        """Return an RPCAuth instance through traversal if available."""
        while hasattr(parent,'aq_parent'):
            RPCAuths = parent.objectValues('RPC Auth')
            for RPCAuth in RPCAuths:
                return RPCAuth
            parent = parent.aq_parent
        return None

    security.declarePublic('idFromTitle')
    def idFromTitle(self, title, metatype, weblog):
        """Return a legal Zope id from a title string.

        FIXME: added obj argument just prior to 9.0 release.  refactor for 1.0
        into base class.
        """

        #  could use string.replace for the ''/- thing
        id = re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', title)).lower()

        if not id:
            # We only do this for thingys in weblogs
            path = '/'.join(weblog.getPhysicalPath())
            thesethings = self.portal_catalog(
                    meta_type = metatype,
                    path = {'query':path, 'level': 0},
                    )
            num = len(thesethings)

            id = re.sub('[^A-Za-z0-9_-]', '', 
                    re.sub(' ', '-', metatype)).lower()
            id += '-' + str(num)

        return id

    security.declarePublic('sendTrackbackPing')
    def sendTrackbackPing(self, entry):
        """Send a trackback ping to the url in the 'trackback' field"""
        data = {}
        data['title'] = entry.Title()
        data['excerpt'] = entry.Description()
        data['url'] = entry.absolute_url()
        data['blog_name'] = self.getParentWeblog(entry).Title()

        try:
            f = urllib.urlopen(entry.getTrackbackURL(), urllib.urlencode(data))
        except:
            self.plone_log('TRACKBACK: urllib tossed an exception')


InitializeClass(QuillsTool)
