#-----------------------------------------------------------------------------
# Purpose:     Migrates a SimpleBlog to a Quills blog.
#
# Author:      Tim Knapp <duffyd@kokorice.org>
#
# Created:     2005/04/03
# SVN-ID:      $Id: migrateSimple2Quills.py,v 1.1 2006/03/10 15:07:05 chaouche Exp $
# Copyright:   (c) 2005 eMerge Technology. All Rights Reserved.
# Licence:     LGPL
#-----------------------------------------------------------------------------

from Products.CMFCore.utils import getToolByName

def createDraft(target, id, title, desc, publishDate, text, topics):
    target.invokeFactory(id=id, type_name='WeblogEntry', title=title,
    description=desc, text=text)
    new_obj = getattr(target, id)
    new_obj.setEffectiveDate(publishDate)
    new_obj.setEntryCategories(topics)

def migrateBlog(self):
    portal = getToolByName(self,'portal_url').getPortalObject()
    
    # These are some values that are used to create
    # the Quills weblog
    quillsBlog = 'blog'
    quillsTitle = 'My Weblog'
    path = quillsBlog + '/drafts/'
    
    # Create the blog based on the values set above
    portal.invokeFactory('Weblog', id=quillsBlog, title=quillsTitle)
    newblog = getattr(portal, quillsBlog)
    
    target = portal.restrictedTraverse(path)
    
    # Find the SimpleBlog we want to migrate
    # (we'll just migrate the first one we find)
    simpleBlogs = portal.portal_catalog.searchResults(portal_type='Blog')
    for simpleBlog in simpleBlogs:
        oldblog = simpleBlog.getObject()
        qtool = portal.quills_tool
        newTopics = []
        blogTopics = newblog.getTopics()

        if oldblog == None or newblog == None:
            return

        for entry in oldblog.objectValues():
        
            # If the current object is a SimpleBlog BlogEntry
            # migrate it
            if entry.meta_type == 'BlogEntry':
                title = entry.Title()
                desc = entry.Description()
                body = entry.getRawBody()
                id = qtool.idFromTitle(title, 'WeblogEntry', newblog)
                
                # Using the '%c' stftime was the only way I could
                # successfully migrate the effective time
                date = entry.getEffectiveDate().strftime('%c')


                # Make sure we don't have the topic already, then add it
                oldBlogCategorys = entry.getField('categories').get(entry)
                for oldBlogCat in oldBlogCategorys:
                    newTopic = qtool.idFromTitle(oldBlogCat, 'WeblogTopic', newblog)
                    if newTopic not in [blogTopic.getId() for blogTopic in blogTopics]:
                        newblog.invokeFactory('WeblogTopic', id=newTopic, title=oldBlogCat)
                        # Update the list of blog topics
                        blogTopics = newblog.getTopics()
                        newTopics.append(newTopic)
                        topics = newTopics
                        newTopics = []

                createDraft(target, id, title, desc, date, body, topics)
        return
