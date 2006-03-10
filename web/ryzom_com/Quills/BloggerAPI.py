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

authOneMethods = [
    'blogger/getTemplate',
    'blogger/setTemplate',
    'blogger/deletePost',
    'blogger/editPost',
    'blogger/newPost',
    'blogger/getRecentPosts'
    ]

def genericBloggerAuthOne(args):
    return args[2],args[3],args

authTwoMethods = [
    'blogger/getUsersBlogs',
    'blogger/getUserInfo'
    ]

def genericBloggerAuthTwo(args):
    return args[1],args[2],args

class BloggerAPI(SimpleItem):
    """http://www.blogger.com/developers/api/1_docs/"""
    security = ClassSecurityInfo()

    def __init__(self, RPCAuth = None):
        if RPCAuth:
            self.setupRPCAuth(RPCAuth)

    security.declarePublic('setupRPCAuth')
    def setupRPCAuth(self, RPCAuth):
        RPCAuth.addAuthProvider(authOneMethods, genericBloggerAuthOne)
        RPCAuth.addAuthProvider(authTwoMethods, genericBloggerAuthTwo)

    security.declarePublic('newPost')
    def newPost(self, appkey, blogid, username, password, content, publish):
        """blogger.newPost makes a new post to a designated blog. 

        Optionally, will publish the blog after making the post. On success,
        it returns the unique ID of the new post.  On error, it will return
        some error message.
        """

        self.plone_log('blogger/newPost', blogid)

        qtool = getToolByName(self, 'quills_tool')
        weblog = qtool.getByUID(blogid)

        post = content.split('\n')
        title = post[0]
        body = '\n'.join(post[1:])

        id = qtool.idFromTitle(title, 'WeblogEntry', weblog)

        if publish:
            effective = DateTime.DateTime()
            archive = getattr(weblog, 'archive')
            dest = archive.createPath(effective)
        else:
            dest = getattr(weblog, 'drafts')

        try:
            dest.invokeFactory('WeblogEntry', id=id, title=title, text=body)
        except Exception, e:
            self.plone_log("Cannot create WeblogEntry: %s" % e)
            return False

        entry = getattr(dest, id)

        # The workflow moves the entry, so get the UID now
        entry_uid = entry.UID()

        wf_tool = getToolByName(self, 'portal_workflow')
        if publish:
            entry.setEffectiveDate(effective)
            wf_tool.doActionFor(entry, 'publish_in_place', 'quills_workflow')

        return entry_uid

    security.declarePublic('editPost')
    def editPost(self, appkey, postid, username, password, content, publish):
        """blogger.editPost changes the contents of a given post. 

        Optionally, will publish the blog the post belongs to after changing
        the post. On success, it returns a boolean true value. On error, it
        will return a fault with an error message.
        """

        self.plone_log('blogger/editPost')

        qtool = getToolByName(self, 'quills_tool')

        entry = qtool.getByUID(postid)
        entry.setText(content, mimetype='text/html')

        if publish:
            wf_tool = getToolByName(self, 'portal_workflow')
            entry.setEffectiveDate(DateTime.DateTime())
            wf_tool.doActionFor(entry, 'publish', None)
        return True

    security.declarePublic('deletePost')
    def deletePost(self, appkey, postid, username, password, publish):
        """Returns true on success, fault on failure"""

        self.plone_log('blogger/deletePost')

        qtool = getToolByName(self, 'quills_tool')

        entry = qtool.getByUID(postid)
        entry.aq_inner.aq_parent.manage_delObjects(entry.getId())

        return True

    security.declarePublic('getRecentPosts')
    def getRecentPosts(self, appkey, blogid, username, password, num):
        """Return 'num' recent posts to specified blog"""

        self.plone_log('blogger/getRecentPosts')

        qtool = getToolByName(self, 'quills_tool')
        weblog = qtool.getByUID(blogid)

        posts = []
        entries = weblog.getFolderListingFolderContents()

        return posts

    security.declarePublic('getUsersBlogs')
    def getUsersBlogs(self, appkey, username, password):
        """Returns information about all the blogs a given user is a member of.

        Data is returned as an array of <struct>'s containing the ID (blogid),
        name (blogName), and URL (url) of each blog.
        """

        self.plone_log('blogger/getUsersBlogs')

        catalog = getToolByName(self, 'portal_catalog')

        results = catalog(meta_type='Weblog', Creator=username)

        blogs = []
        for item in results:
            o = item.getObject()
            self.plone_log(o.UID())
            blogs.append(
                    {'url': o.absolute_url(),
                     'blogid' : o.UID(),
                     'blogName' : o.title_or_id()}
                    )

        return blogs

    security.declarePublic('getUserInfo')
    def getUserInfo(self, appkey, username, password):
        """Returns a struct containing userinfo

        userid, firstname, lastname, nickname, email, and url.
        """

        self.plone_log('blogger/getUserInfo')

        membership=getToolByName(self, 'portal_membership')

        info={'name':'no name',
              'email':'no email',
              'userid':'no user id',
              'firstname':'no first name',
              'lastname':'no last name',
              'url':'no url'
             }

        member=membership.getAuthenticatedMember()

        if member:
            for key,value in info.items():
                info[key] = getattr(member,key,None) or value

        return info

    security.declarePublic('getTemplate')
    def getTemplate(self):
        """Returns text of the main or archive index template for a blog."""

        # Not implementing
        self.plone_log('blogger/getTemplate')
        pass

    security.declarePublic('setTemplate')
    def setTemplate(self):
        """Changes the template for a given blog. 

        Can change either main or archive index template.
        """

        # Not implementing
        self.plone_log('blogger/setTemplate')
        pass

