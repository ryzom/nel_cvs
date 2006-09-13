#
# Authors: Brian Skahan <bskahan@etria.com>
#          Tom von Schwerdtner <tvon@etria.com>
#
# Copyright 2004-2005, Etria, LLP
# # This file is part of Quills
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



from Products.Archetypes.Marshall import RFC822Marshaller
from Products.CMFCore import CMFCorePermissions
from config import PROJECTNAME
import DateTime

base_schema = BaseSchema.copy()
WeblogEntrySchema = base_schema + Schema((

    TextField('description',
        searchable=1,
        accessor="Description",
        widget=TextAreaWidget(
            label='Excerpt',
            description="""A brief introduction for this entry.
            """,
            label_msgid="label_description",
            description_msgid="help_description",
            i18n_domain="quills"),
        ),
    TextField('text',
        searchable=1,
        default_output_type='text/html',
        allowable_content_types=(
            'text/restructured',
            'text/plain',
            'text/html',
            ),
        widget=RichWidget(label='Entry Text',
            rows=30,
            label_msgid="label_text",
            i18n_domain="quills"),
        ),

    LinesField('entryCategories',
           default = 'general',
           widget=MultiSelectionWidget(label='Categories'),
           vocabulary='getWeblogCategories',
           index='KeywordIndex',
        ),

    StringField('trackbackURL',
        widget=StringWidget(label='Trackback',
            description="If you want to send a trackback ping to another blog,\
                    enter the trackback url here"),
        ),

    # Someday someone will start trackback spamming.  Before that day comes, we
    # need to setup a nice editing interface for this so weblog folks can
    # remove unwanted trackback pings.
    LinesField('trackbackPings',
           widget=LinesWidget(label='Trackback Pings',
                description="Trackback pings sent to this entry.  If you get \
                        any unwanted trackback pings, you can remove them \
                        here.  Otherwise, do not edit this field.",
           visible= {'edit' : 'hidden',}
            ),
        ),

    # I don't like this hack too much, but the result is 'nice'
    StringField('creatorFullname',
        widget=StringWidget( visible= {'edit' : 'hidden',}),
        ),

    ),
    marshall=RFC822Marshaller(),
    )


class WeblogEntry(BaseContent):
    """Basic Weblog Entry"""
    schema = WeblogEntrySchema
    archetype_name = "Weblog Entry"
    meta_type = 'WeblogEntry'
    content_icon = 'weblogentry_icon.gif'
    global_allow = False
    _at_rename_after_creation = True
    default_view  = 'weblogentry_view'
    immediate_view = 'weblogentry_view'

    actions = (
        { 'id': 'view',
          'name': 'View',
          'action': 'string:${object_url}/weblogentry_view',
          'permissions': (CMFCorePermissions.View,) 
        },)

    def __init__(self, oid, **kwargs):
        self.schema['allowDiscussion'].default = True
#        self.schema['id'].widget.visible = {'edit': 'invisible'}
        BaseContent.__init__(self, oid, **kwargs)

    # Set start/end to effective to use portal_calendar for a blog archive
    def start(self):
        return self.effective()

    def end(self):
        return self.effective()

    def subject(self):
        result = []

        weblog = self.quills_tool.getParentWeblog(self)

        #topics = self.quills_tool.getWeblogTopics(weblog)
        topics = weblog.getTopics()

        for topic in topics:
            result.append(topic.Title())

        return result

#    def setTitle(self, value, **kwargs):
#        self.getField('title').set(self, value, **kwargs)
#        if value:
#            weblog = self.quills_tool.getParentWeblog(self)
#            self.setId(self.quills_tool.idFromTitle(value, 'WeblogEntry',
#                weblog))

    def getArchiveUrl(self):
        """Build a /YYYY/MM/DD/entry path for referencing posts"""
        # FIXME: Is this needed anymore?
        year = str(DateTime.DateTime(str(self.getEffectiveDate())).year())
        month = DateTime.DateTime(str(self.getEffectiveDate())).mm()
        day = DateTime.DateTime(str(self.getEffectiveDate())).dd()
        path = 'archive/' + year + '/' + month + '/' + day + '/' + self.id
        return path

    def getAbsoluteArchiveUrl(self):
        """Build a /YYYY/MM/DD/entry path for referencing posts"""

        # FIXME: Is this needed anymore?

        parent = self.quills_tool.getParentWeblog(self)
        path =  parent.absolute_url() + '/' + self.getArchiveUrl()
        return path

    def getTrackbacks(self):
        """Return a list of trackback dicts"""
        trackbacks = self.getTrackbackPings()
        result = []
        for tb in trackbacks:
            values = tb.split('|')
            tb_dict = {
                    'url': values[0],
                    'title' : values[1],
                    'excerpt' : values[2],
                    'blog_name' : values[3],
                    'added' : values[4]
                    }
            result.append(tb_dict)

        return result

    def addTrackback(self, url, title, excerpt, blog_name, added=None):
        """Add a trackback line to this entry.

        The passed variable can be the request containing the trackback
        information or a dict with the appropriate trackback keys.
        """

        if added is None:
            added = DateTime.DateTime()

        s = '|'.join((url, title, excerpt, blog_name, str(added)))

        current_trackback = self.getTrackbackPings()

        updated_trackback = []
        for line in current_trackback:
            updated_trackback.append(line)

        updated_trackback.append(s)

        self.setTrackbackPings(updated_trackback)

        return True

    def getWeblogCategories(self):
        """Return a list of the Categories this Entry is assigned"""

        result = DisplayList()

        weblog = self.quills_tool.getParentWeblog(self)

        #topics = self.quills_tool.getWeblogTopics(weblog)
        topics = weblog.getTopics()

        for topic in topics:
            result.add(topic.getId(), topic.Title())

        return result


registerType(WeblogEntry, PROJECTNAME)
