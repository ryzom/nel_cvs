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

try:
    from Products.LinguaPlone.public import *
except ImportError:
    from Products.Archetypes.public import *    

from AccessControl import ClassSecurityInfo
#from Products.Archetypes.public import *

from Products.Archetypes.Marshall import PrimaryFieldMarshaller
from Products.CMFCore import CMFCorePermissions
from DateTime import DateTime
import MetaWeblogAPI
import BloggerAPI
#import MovableTypeAPI

from config import PROJECTNAME

WeblogSchema = BaseFolderSchema + Schema((

    # This is sort of cheating.  We are stealing the Dublin Core 'description'
    # field for our purposes, but then I don't seen any reason to duplicate the
    # information.
    TextField('description',
        searchable=1,
        accessor="Description",
        widget=TextAreaWidget(
            label='Description',
            description="""A brief description of this weblog.  This text will
            be displayed at the top of the page before any weblog entries.""",
            label_msgid="label_description",
            description_msgid="help_description",
            i18n_domain="plone"),
        ),
        
   	TextField('menuDescription',
        searchable=0,
        default="all news topic",
		widget=TextAreaWidget(
			label='Menu',
			description="""The string that will be display to present the menu""",),
		),
		
	TextField('menuFirstItem',
        searchable=0,
        default="trier par rubrique",
		widget=TextAreaWidget(
			label='Menu',
			description="""The string that will be display in the fast topic changing menu""",),
		),

    ### 
    ## Configuration Options
    ###
    BooleanField('addNewsButton',
        default = False,
        widget=BooleanWidget(label='Add a Button --Submit a News-- ?',
            description="""When enabled, a Button --Submit a News appear--.""",
            ),
        schemata='configuration'
        ), 
    BooleanField('onlyExcerptInWeblogView',
        default = False,
        widget=BooleanWidget(label='Only Show Excerpts in Weblog?',
            description="""When enabled, only the excerpt of an entry will be
            displayed in the main weblog view.  If no excerpt is provided then
            only the title will be displayed.
            """,
            ),
        schemata='configuration'
        ),
    IntegerField('entriesPerPage',
        default = 20,
        widget=IntegerWidget(label='Entries Per Page',
            description="Select the number of weblog entries you would like \
                    to display on the front page and any other batched pages.",
            ),
        schemata='configuration'
        ),

    BooleanField('showTopicImagesInWeblogView',
        default = 1,
        widget=BooleanWidget(label='Show Topic Images In Weblog View?',
            description="This controls the dispaly of topic images in the \
                    weblog view.",
            ),
        schemata='configuration'
        ),

    BooleanField('trackbackEnabled',
        default = 1,
        widget=BooleanWidget(label='Enable the sending/receiving of trackback \
                pings?',
            ),
        schemata='configuration'
        ),
    ),
    
	BooleanField('commentsEnabled',
		default = True,
		widget=BooleanWidget(label='Enable comments of weblog entries',
			description="""When disabled, the comments are disabled""",
			),
		schemata='configuration'
		),

    marshall=PrimaryFieldMarshaller(),
    )

class Weblog(BaseFolder):
    """Weblog object"""

    schema = WeblogSchema
    security = ClassSecurityInfo()
    archetype_name = "Weblog"
    meta_type = 'Weblog'
    content_icon = 'weblog_icon.gif'
    _at_rename_after_creation = True
    allowed_content_types = (
            'WeblogEntry',
            'WeblogTopic',
            )

    default_view = 'weblog_view'
    immediate_view = 'weblog_view'
    actions = (
        { 'id': 'view',
          'name': 'View',
          'action': 'string:${folder_url}/weblog_view',
          'permissions': (CMFCorePermissions.View,) },
        { 'id': 'topics',
          'name': 'Topics',
          'action': 'string:${folder_url}/weblogtopics_view',
          'permissions': (CMFCorePermissions.ModifyPortalContent,) },
        { 'id': 'archive',
          'name': 'Archive',
          'action': 'string:${folder_url}/archive/weblogarchive_view',
          'permissions': (CMFCorePermissions.ModifyPortalContent,) },
          {
          'id': 'local_roles',
          'name': 'Sharing',
          'action': 'string:${object_url}/folder_localrole_form',
          'permissions': (CMFCorePermissions.ManageProperties,),
          },
        { 'id': 'drafts',
          'name': 'Drafts',
          'action': 'string:${folder_url}/weblogdrafts_view',
          'permissions': (CMFCorePermissions.ModifyPortalContent,) },
        )

    def initializeArchetype(self, **kwargs):
        BaseFolder.initializeArchetype(self, **kwargs)
        RPCAuth = self.quills_tool.findRPCAuth(self)

        # Setup the MetaWeblog API
        if not hasattr(self.aq_inner.aq_explicit, 'metaWeblog'):
            self.metaWeblog = MetaWeblogAPI.MetaWeblogAPI().__of__(self)
            self.metaWeblog.setupRPCAuth(RPCAuth)


        # Setup the Blogger API
        if not hasattr(self.aq_inner.aq_explicit, 'blogger'):
            self.blogger = BloggerAPI.BloggerAPI().__of__(self)
            self.blogger.setupRPCAuth(RPCAuth)


        # Create a 'general' category
        if not hasattr(self.aq_inner.aq_explicit, 'general'):
            # Create a 'general' category
            self.invokeFactory('WeblogTopic',id='general')
            gen = getattr(self.aq_inner.aq_explicit, 'general')
            gen.setTitle('General')

        # create archive root
        if not hasattr(self.aq_inner.aq_explicit, 'archive'):
            # create archive root
            self.portal_types.constructContent('WeblogArchive',
                                               self,
                                               'archive',
                                               title='Archive',
                                               archive_type='root')

        # create drafts folder
        if not hasattr(self.aq_inner.aq_explicit, 'drafts'):
            # create drafts folder
            self.portal_types.constructContent('WeblogDrafts',
                                               self,
                                               'drafts',
                                               title='Drafts')

    # XXX: s/Topics/Categories/
    security.declarePublic('getTopics')
    def getTopics(self):
        """Return topics/categories instances for this weblog."""

        results = self.getFolderListingFolderContents(
                        contentFilter={'portal_type' : 'WeblogTopic','sort_on' : 'sortable_title',},
                        suppressHiddenFiles=1)
        return results

    # XXX: Fixup for removal of 'archive' folder
    security.declarePublic('getArchives')
    def getArchives(self):
        """Return WeblogArchive objects for each year."""

        # Currently we have the 'archive' folder only, but this will change
        # soon(tm)
        soon_to_go_away = getattr(self.aq_inner.aq_explicit, 'archive')
        archives = soon_to_go_away.getFolderListingFolderContents(
                contentFilter={'portal_type': 'WeblogArchive'},
                suppressHiddenFiles=1)

        return archives

    # XXX: Fixup to use getArchives (which returns a list)
    security.declarePublic('getEntries')
    def getEntries(self):
        """Return all entries for this weblog.

        XXX: This should be optimized so it does not do a getObject on all the
        entries in the weblog."""

        #archives = self.getArchives()
        archive = getattr(self.aq_inner.aq_explicit, 'archive')
        results = {}
        epp = self.getEntriesPerPage()
        lazy_entries = []
        lazy_entries = archive.getLazyEntries()
        for lazy_entry in lazy_entries[:epp]:
            res = lazy_entry.getObject()
            date = DateTime(res.EffectiveDate()).strftime('%Y-%m-%d 00:00:00')
            try:
                if results[date]:
                    # Add the entry to the top of the list for that day
                    results[date].append(res)
            except:
                results[date] = [res,]

        return results


    # XXX: s/Topics/Categories/ (reall, this will be removed)
    security.declarePublic('getAvailableTopics')
    def getAvailableTopics(self):
        return self.getTopics()

    security.declarePublic('getTopicById')
    def getTopicById(self, id):
        """Given a topic id, return it's title"""
        topics = self.getTopics()
        for topic in topics:
            if topic.getId() == id:
                return topic

        return None

    def getDrafts(self):
        """Return a list of lazy_maps for all WeblogEntries in this Weblog that are
        in the 'draft' review_state."""

        path = '/'.join(self.getPhysicalPath())
        results = self.portal_catalog(
                meta_type=['WeblogEntry',],
                path={'query':path, 'level': 0},
                sort_on = 'effective',
                sort_order = 'reverse',
                review_state = 'draft')

        return results

registerType(Weblog, PROJECTNAME)
