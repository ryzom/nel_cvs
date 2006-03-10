"""
$Id: test_weblog.py,v 1.1 2006/03/10 15:07:06 chaouche Exp $
"""

import os, sys
if __name__ == "__main__":
    execfile(os.path.join(sys.path[0], "framework.py"))

from Testing import ZopeTestCase
from Products.Quills.tests import QuillsTestCase

from Products.Quills import config


class TestWeblog(QuillsTestCase.QuillsTestCase):

    def afterSetUp(self):
        self.setRoles(["Manager"])
        self.folder.invokeFactory("Weblog", id="weblog", title="Test Weblog")
        self.weblog = getattr(self.folder, "weblog")

    def testInitializeArchetype(self):
        general = getattr(self.weblog, "general")
        self.failUnless(general.Title() == "General")
        archive = getattr(self.weblog, "archive")
        self.failUnless(archive.Title() == "Archive")
        drafts = getattr(self.weblog, "drafts")
        self.failUnless(drafts.Title() == "Drafts")

    def testGetTopics(self):
        self.failUnless(len(self.weblog.getTopics()) == 1)
        self.assertEquals(
                [topic.Title() for topic in self.weblog.getTopics()],
                ['General',])

        self.weblog.invokeFactory('WeblogTopic', id='testing', title="Testing")
        self.assertEquals(
                [topic.Title() for topic in self.weblog.getTopics()],
                ['General', 'Testing'])

        # Make sure we only get WeblogTopics back
        self.weblog.invokeFactory('WeblogEntry', id='test-entry',
                title="Test Entry")
        self.assertEquals(
                [topic.Title() for topic in self.weblog.getTopics()],
                ['General', 'Testing'])


def test_suite():
    from unittest import TestSuite, makeSuite
    suite = TestSuite()
    suite.addTest(makeSuite(TestWeblog))
    return suite

if __name__ == '__main__':
    framework()
