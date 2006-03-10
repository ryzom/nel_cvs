"""
$Id: test_quills.py,v 1.1 2006/03/10 15:07:06 chaouche Exp $
"""

import os, sys
if __name__ == "__main__":
    execfile(os.path.join(sys.path[0], "framework.py"))

from Testing import ZopeTestCase
from Products.Quills.tests import QuillsTestCase

from Products.Quills import config


class TestPortalTypes(QuillsTestCase.QuillsTestCase):

    def afterSetUp(self):
        self.types = self.portal.portal_types.objectIds()

    def testPortalTypesExists(self):
        types = [
            "Weblog",
            "WeblogArchive",
            "WeblogEntry",
            "WeblogTopic",
        ]
        for t in types:
            self.failUnless(t in self.types, "Type not installed: %s" % t)

    def testQuillsAllowedTypes(self):
        u"""Test the Weblog allowed content types.

        Only WeblogEntry and WeblogTopic should be available.
        """
        weblog = self.portal.portal_types.getTypeInfo('Weblog')

        self.failUnless("WeblogEntry" in weblog.allowed_content_types)
        self.failUnless("WeblogTopic" in weblog.allowed_content_types)

        self.failUnlessEqual(len(weblog.allowed_content_types), 2)


class TestFolderContainment(QuillsTestCase.QuillsTestCase):

    def afterSetUp(self):
        self.setRoles(["Manager"])
        self.folder.invokeFactory("Weblog", id="weblog", title="Test Weblog")

    def testQuillsContainment(self):
        """Test adding objects that we should be able to add to a Weblog."""
        self.folder.weblog.invokeFactory("WeblogEntry", id="entry")
        self.failUnless("entry" in self.folder.weblog.objectIds())
        self.folder.weblog.invokeFactory("WeblogTopic", id="stuff")
        self.failUnless("stuff" in self.folder.weblog.objectIds())


def test_suite():
    from unittest import TestSuite, makeSuite
    suite = TestSuite()
    suite.addTest(makeSuite(TestPortalTypes))
    suite.addTest(makeSuite(TestFolderContainment))
    return suite

if __name__ == '__main__':
    framework()
