"""Quills doctest runner.
"""

__author__ = 'Tom von Schwerdtner'
__docformat__ = 'restructuredtext'

import os, sys
if __name__ == '__main__':
    execfile(os.path.join(sys.path[0], 'framework.py'))

from Testing import ZopeTestCase

# a list of dotted paths to modules which contains doc tests
DOCTEST_MODULES = (
    'Products.Quills.Weblog',
    'Products.Quills.tests.test_doctests',
    )

def doctest_test():
    """Sample doctest.

    Unlike z3, any file that has doctests needs to be included in
    DOCTEST_MODULES above.

        >>> True
        True

    """

from Products.Quills.tests.QuillsTestCase import QuillsTestCase
#from Products.Archetypes.tests.doctestcase import ZopeDocTestSuite

#def test_suite():
#    suite = ZopeDocTestSuite(test_class=QuillsTestCase,
#                             extraglobs={},
#                             *DOCTEST_MODULES
#                             )
#    return suite

if __name__ == '__main__':
    framework()

