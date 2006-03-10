"""
$Id: test_weblogentry.py,v 1.1 2006/03/10 15:07:06 chaouche Exp $
"""

import os, sys
if __name__ == "__main__":
    execfile(os.path.join(sys.path[0], "framework.py"))

from Testing import ZopeTestCase
from Products.Quills.tests import QuillsTestCase

from Products.Quills import config


class TestWeblogEntry(QuillsTestCase.QuillsTestCase):

    def afterSetUp(self):
        self.setRoles(["Manager"])

        self.folder.invokeFactory("Weblog", id="weblog", title="Test Weblog")
        self.weblog = getattr(self.folder, "weblog")

        self.weblog.invokeFactory("WeblogEntry", "entry",
                                  title="Test Entry")


    def testEntry(self):

        from Products.CMFCore import CMFCorePermissions
        from Products.Quills.WeblogEntry import WeblogEntrySchema

        entry = getattr(self.weblog, "entry")
        self.failUnless(entry.id == "entry")
        self.failUnless(entry.title == "Test Entry")
        self.failUnless(entry.schema == WeblogEntrySchema)
        self.failUnless(entry.archetype_name == "Weblog Entry")
        self.failUnless(entry.meta_type == 'WeblogEntry')
        self.failUnless(entry.content_icon == 'weblogentry_icon.gif')
        self.failUnless(entry.global_allow == False)
        self.failUnless(entry.default_view == 'weblogentry_view')
        self.failUnless(entry.immediate_view == 'weblogentry_view')
        self.failUnless(entry.schema['allowDiscussion'].default == True)
        self.failUnless(entry.schema['id'].widget.visible == \
                                                        {'edit': 'invisible'})

        self.failUnless(entry.start() == entry.end(),
                        "start and end should be the same")

        # XXX: How did the entry get assigned to general?
        cats = [cat for cat in entry.getWeblogCategories()]
        self.failUnlessEqual(cats, ['general'])

        # XXX: this looks a little redundant....
        self.failUnless(entry.subject() == ['General'])

        tbs = entry.getTrackbacks()
        self.failUnlessEqual(tbs, [])

        from datetime import datetime
        self.failUnless(entry.addTrackback("http://example.com", "Example",
                           "Shouldn't excerpt be a keyword arg",
                           "The Example Blog"))

        tbs = entry.getTrackbacks()
        self.failUnlessEqual(tbs[0]['url'], 'http://example.com')



def test_suite():
    from unittest import TestSuite, makeSuite
    suite = TestSuite()
    suite.addTest(makeSuite(TestWeblogEntry))
    return suite

if __name__ == '__main__':
    framework()
