# -*- coding: UTF-8 -*-
"""
$Id: test_BloggerAPI.py,v 1.1 2006/03/10 15:07:06 chaouche Exp $
"""

import os, sys
if __name__ == '__main__':
    execfile(os.path.join(sys.path[0], 'framework.py'))

from Testing import ZopeTestCase
from Products.Quills.tests import QuillsTestCase
from Products.Quills import config

username = ZopeTestCase.user_name
password = ZopeTestCase.user_password
appid = None

class TestCaseMixin(QuillsTestCase.QuillsTestCase):

    def afterSetUp(self):
        self.folder.invokeFactory("Weblog", id="weblog", title="My Weblog")
        self.qtool = self.folder.quills_tool
        self.weblog = self.folder.weblog


class TestBloggerAPI(TestCaseMixin):

    def testGetUsersBlogs(self):
        weblogs = self.weblog.blogger.getUsersBlogs(appid, username, password)

        expected = [ 
            {'url': 'http://nohost/plone/Members/%s/weblog' % username,
            'blogid':  self.weblog.UID(),
            'blogName': 'My Weblog'}]

        self.failUnlessEqual(expected, weblogs)

    def testNewPostPublish(self):
        """Test creating a BloggerAPI entry and publishing it"""
        self.login(username)
        blogid = self.weblog.UID()

        content = """This is my content title

        Dig it.
        """

        result = self.weblog.blogger.newPost(appid, blogid, username, password,
                content, True)

        entry = self.qtool.getByUID(result)

        import DateTime
        dt = DateTime.DateTime()

        expected_path = ('weblog', 'archive', 
                str(dt.year()),
                dt.mm(),
                dt.dd(), 
                'this-is-my-content-title')

        self.failUnlessEqual(expected_path, entry.getPhysicalPath()[-6:])

        self.failUnlessEqual('<p>Dig it.</p>', entry.getText())

    def testNewPostNoPublish(self):
        """Test creating a BloggerAPI entry but not publishing it"""
        blogid = self.weblog.UID()
        content = """This is my blogger title

        This is the body!
        """

        result = self.weblog.blogger.newPost(appid, blogid, username, password,
                content, False)

        entry = self.qtool.getByUID(result)

        expected_path = ('weblog', 'drafts', 'this-is-my-blogger-title')
        self.failUnlessEqual(expected_path, entry.getPhysicalPath()[-3:])

        self.failUnlessEqual('<p>This is the body!</p>', entry.getText())


def test_suite():
    from unittest import TestSuite, makeSuite
    suite = TestSuite()
    suite.addTest(makeSuite(TestBloggerAPI))
    return suite

if __name__ == '__main__':
    framework()
