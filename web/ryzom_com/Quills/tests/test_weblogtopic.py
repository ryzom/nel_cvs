"""
$Id: test_weblogtopic.py,v 1.1 2006/03/10 15:07:06 chaouche Exp $
"""

import os, sys
if __name__ == "__main__":
    execfile(os.path.join(sys.path[0], "framework.py"))

from Testing import ZopeTestCase
from Products.Quills.tests import QuillsTestCase

from Products.Quills import config


class TestWeblogTopic(QuillsTestCase.QuillsTestCase):

    def afterSetUp(self):
        self.setRoles(["Manager"])
        self.folder.invokeFactory("Weblog", id="weblog", title="Test Weblog")
        self.weblog = getattr(self.folder, "weblog")

    def testTopic(self):
        self.failUnless(len(self.weblog.getTopics()) == 1)
        self.assertEquals(
                [topic.Title() for topic in self.weblog.getTopics()],
                ['General',])

        general = getattr(self.weblog, "general")

        self.assert_(general.id, '')
        self.assert_(general.default_topic_image, 'default-topic-icon.jpg')
        self.failUnless(general.schema['allowDiscussion'].default == 0)
        self.failUnless(general.global_allow == 0)
        self.failUnless(general.archetype_name == "Topic")
        self.failUnless(general.topicImage == "")


        self.weblog.invokeFactory('WeblogTopic', id='testing', title="Testing")
        self.assertEquals(
                [topic.Title() for topic in self.weblog.getTopics()],
                ['General', 'Testing'])


def test_suite():
    from unittest import TestSuite, makeSuite
    suite = TestSuite()
    suite.addTest(makeSuite(TestWeblogTopic))
    return suite

if __name__ == '__main__':
    framework()
