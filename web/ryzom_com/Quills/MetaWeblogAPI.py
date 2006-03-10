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


from OFS.SimpleItem import SimpleItem
from Products.CMFCore.utils import getToolByName
from Products.CMFDefault.utils import parseHeadersBody
from AccessControl import ClassSecurityInfo
import DateTime

authMethods = [
    'metaWeblog/getPost',
    'metaWeblog/deletePost',
    'metaWeblog/editPost',
    'metaWeblog/newPost',
    'metaWeblog/getRecentPosts',
    'metaWeblog/getUsersBlogs',
    'metaWeblog/getCategories'
    ]

def genericMetaWeblogAuth(args):
    return args[1], args[2], args


class MetaWeblogAPI(SimpleItem):
    """http://www.metaWeblog.com/developers/api/1_docs/"""

    security = ClassSecurityInfo()

    def __init__(self, RPCAuth = None):
        if RPCAuth:
            self.setupRPCAuth(RPCAuth)

    security.declarePublic('setupRPCAuth')
    def setupRPCAuth(self, RPCAuth):
        RPCAuth.addAuthProvider(authMethods, genericMetaWeblogAuth)

    security.declarePublic('newPost')
    def newPost(self, blogid, username, password, struct, publish):
        """Some Stuff"""
        self.plone_log('metaWeblog/newPost')

        qtool = getToolByName(self, 'quills_tool')

        weblog = qtool.getByUID(blogid)

        body  = struct.get('description', struct.get('Description'))
        title = struct.get('title', struct.get('Title'))

        id = qtool.idFromTitle(title)

        # Stick it directly in the drafts folder
        drafts = getattr(weblog, 'drafts')
        drafts.invokeFactory('WeblogEntry', id = id, title = title)

        entry = getattr(drafts, id)

        entry.setBody(body, mimetype='text/html')

        # The workflow moves the entry, so get the UID now
        entry_uid = entry.UID()

        wf_tool = getToolByName(self, 'portal_workflow')
        if publish:
            entry.setEffectiveDate(DateTime.DateTime())
            wf_tool.doActionFor(entry, 'make_published', None)

        return entry_uid

    security.declarePublic('editPost')
    def editPost(self, postid, username, password, struct, publish):
        """Implement MetaWeblog editPost"""

        self.plone_log('metaWeblog/editPost')

        qtool = getToolByName(self, 'quills_tool')

        entry = qtool.getByUID(postid)

        body  = struct.get('description', struct.get('Description'))
        title = struct.get('title', struct.get('Title'))

        entry.setBody(body, mimetype='text/html')
        entry.setTitle(title)
 
        if publish:
            wf_tool = getToolByName(self, 'portal_workflow')
            entry.setEffectiveDate(DateTime.DateTime())
            wf_tool.doActionFor(entry, 'make_published', None)

        return True

    security.declarePublic('getPost')
    def getPost(self, postid, username, password):
        "Return a post I suppose"
        self.plone_log('metaWeblog/getPost')

        qtool = getToolByName(self, 'quills_tool')

        obj = qtool.getByUID(postid)

        entry = {'title': obj.Title(),
                 'description' : obj.getBody(),
                 'link' : obj.absolute_url()
                }

        return entry

    security.declarePublic('getCategories')
    def getCategories(self, blogid, username, password):
        """Returns a struct containing description, htmlUrl and rssUrl"""
        self.plone_log('metaWeblog/getCategories')

        qtool = getToolByName(self, 'quills_tool')

        weblog = qtool.getByUID(blogid)

        topics = weblog.getAvailableTopics()

        categories = []
        for topic in topics:
            categories.append(
                {'description' : topic.title_or_id(),
                'htmlUrl' : topic.absolute_url(),
                'rssUrl' : topic.absolute_url()
                })

        self.plone_log('metaWeblog/getCategories: returning ', categories)
        return categories

    security.declarePublic('deletePost')
    def deletePost(self, postid, username, password, publish):
        "Returns true on success, fault on failure"
        self.plone_log('metaWeblog/deletePost')

        qtool = getToolByName(self, 'quills_tool')

        entry = qtool.getByUID(postid)

        entry.aq_inner.aq_parent.manage_delObjects(entry.getId())

        return True

    security.declarePublic('getRecentPosts')
    def getRecentPosts(self, blogid, username, password, num):
        """Return 'num' recent posts to specified blog, 

        returns a struct: The three basic elements are title, link and
        description. For blogging tools that don't support titles and links,
        the description element holds what the Blogger API refers to as
        'content'.
        """

        self.plone_log('metaWeblog/getRecentPosts')

        catalog = getToolByName(self, 'portal_catalog')
        results = catalog(meta_type='WeblogEntry')

        posts = []
        for item in results:
            obj = item.getObject()
            posts.append(
                {'title': obj.Title(),
                 'description' : obj.getBody(),
                 'link' : obj.absolute_url()
                }
            )
        return posts

    security.declarePublic('getUsersBlogs')
    def getUsersBlogs(self, username, password):
        """TODO: getUsersBlogs"""

        self.plone_log('metaWeblog/getUsersBlogs')

        catalog = getToolByName(self, 'portal_catalog')

        results = catalog(meta_type='Weblog', Creator=username)

        blogs = []
        for item in results:
            o = item.getObject()
            blogs.append(
                    {'url': o.absolute_url(),
                     'blogid' : o.UID(),
                     'blogName' : o.title_or_id()}
                    )

        return blogs

    security.declarePublic('getUserInfo')
    def getUserInfo(self, appkey, username, password):
        """Returns returns a struct containing userinfo

        userid, firstname, lastname, nickname, email, and url.
        """

        self.plone_log('metaWeblog/getUserInfo')

        membership=getToolByName(self, 'portal_membership')
        info={'name':'no name','email':'no email','userid':'no user id'
               ,'firstname':'no first name','lastname':'no last name'
               ,'url':'no url'}
        member=membership.getAuthenticatedMember()
        if member:
            for key,value in info.items():
                info[key] = getattr(member,key,None) or value
        return info
