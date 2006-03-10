#
# Purpose:     Migrates a COREBlog instance to a Quills blog.
#
# Author:      Tom Lazar <tom@tomster.org>
#              based on Tim Knapp's <duffyd@kokorice.org>
#              script migrateSimple2Quills.py
#
# Created:     2005/04/19
# SVN-ID:      $Id: migrateCOREBlog2Quills.py,v 1.1 2006/03/10 15:07:05 chaouche Exp $
# Licence:     LGPL
#
# This file is part of Quills
###############################################################################

from Products.CMFCore.utils import getToolByName

def createDraft(self, target, id, title, desc, publishDate, excerpt, text,
                topics):

    target.invokeFactory(id=id, type_name='WeblogEntry', title=title,
    description=desc, text=text)
    new_obj = getattr(target, id)
    new_obj.setEffectiveDate(publishDate)
    new_obj.setModificationDate(publishDate)
    new_obj.setEntryCategories(topics)
    new_obj.setDescription(excerpt)
    new_obj.setText(text)

    return new_obj


def addTopic(self, newblog, newId, newName, existingTopics):
    """ add a category to the blog - but only, if it doesn't exist yet."""

    if newId not in [blogTopic.getId() for blogTopic in existingTopics]:
        self.plone_log("Adding category [%s]" %newId)
        newblog.invokeFactory('WeblogTopic', id=newId, title=newName)


def mod_rewrite_rule(self, cb_entry, id, path):

    date = cb_entry.date_created()
    year = str(date.year())
    month = str(date.month()).zfill(2)
    day = str(date.day()).zfill(2)

    dest_url = "/" + path + year + "/" + month + "/" + day + "/" + id
    source_url = "^/blog/" + cb_entry.id + ".*"
    rule = "RewriteRule " + source_url + " " + dest_url

    return rule


def addComment(self, dtool, quills_item, coreblog_item):

    dtool.getDiscussionFor(quills_item)
    tb = quills_item.talkback
    reply_id = tb.createReply(title=coreblog_item.title,
                              text=coreblog_item.body,
                              Creator=coreblog_item.author)
    reply = tb.getReply(reply_id)

    date = coreblog_item.date_created().strftime('%c')
    self.plone_log("== Adding COMMENT [" + coreblog_item.title + "]")

    reply.setEffectiveDate(date)
    reply.setModificationDate(date)

    return reply


def migrateBlog(self):
    portal = getToolByName(self,'portal_url').getPortalObject()

    # These are some values that are used to create
    # the Quills weblog
    quillsBlog = 'blog'
    drafts_path = quillsBlog + '/drafts/'
    archive_path = quillsBlog + '/archive/'

    coreBlog = 'coreblog'

    rewrite_rules = ""
    target = portal.restrictedTraverse(drafts_path)

    # Create the blog based on the values set above
    newblog = getattr(portal, quillsBlog)
    self.plone_log("Found new blog [%s]" %newblog.title)
    qtool = portal.quills_tool
    dtool = portal.portal_discussion
    blogTopics = newblog.getTopics()

    coreblog = getattr(portal, coreBlog)
    self.plone_log("... and CoreBLOG [%s]" %coreblog.title)

    # since COREBlog uses numerical ids we convert their title to ids
    # and use those then as match for exising categories
    coreblog_categories = coreblog.category_list()
    coreblog_category_ids = [qtool.idFromTitle(category.name, 'WeblogEntry', newblog) \
                             for category in coreblog_categories]

    # then we add all categories from the COREBlog instance, that don't
    # exist yet in the Quills instance:

    for i in range(0, len(coreblog_categories)):
        addTopic(self, newblog, coreblog_category_ids[i],
                 coreblog_categories[i].name, blogTopics)

    entry_list = coreblog.entry_list

    self.plone_log("----- Starting migration -----")
    for i in entry_list:

        entry = coreblog.get_entry(i)
        title = entry.title
        self.plone_log("### Found entry " + str(i) + " [" + title + "]")
        desc = entry.description
        excerpt = entry.subtitle
        body = entry.rendered_body + entry.extend
        new_id = qtool.idFromTitle(title, 'WeblogEntry', newblog)
        date = entry.date_created().strftime('%c')
        topics = [coreblog_category_ids[entry.category[0] - 1]]

        rewrite_rules = rewrite_rules + "\r" + mod_rewrite_rule(self, entry,
                                                                new_id,
                                                                archive_path)

        # if we already added a COREBLog entry with the same title, we append
        # the numerical id to the id of the Quills WeblogEntry to avoid a
        # conflict.
        try:
            new_entry = createDraft(self, target, new_id, title, desc, date,
                                    excerpt, body, topics)
        except:
            new_entry = createDraft(self, target, new_id + str(i),
                                    title + str(i), desc, date, excerpt, body,
                                    topics)

        # now we migrate the comments
        for c in entry.comments:
            comment = coreblog.getComment(c)
            addComment(self, dtool, new_entry, comment)

        # and finally the trackbacks
        for tb in entry.trackbacks:
            trackback = coreblog.getTrackback(tb)

            title = trackback.title.decode('latin-1')
            excerpt = trackback.excerpt.decode('latin-1')
            blog_name = trackback.blog_name.decode('latin-1')
            url = trackback.url.decode('latin-1')

            self.plone_log("## Adding trackback " + title)
            new_entry.addTrackback(url, 
                                  title, 
                                  excerpt,
                                  blog_name,
                                  trackback.date_created())

    self.plone_log("----- Finito! -----")

    return rewrite_rules
