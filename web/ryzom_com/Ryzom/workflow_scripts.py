from Products.CMFCore.utils import getToolByName

def moveToWeblogRoot(self, info):
    o = info.object
    weblog = getToolByName(self, 'quills_tool').getParentWeblog(self)
    origin = o.aq_parent

    o_id = o.getId()
    weblog.manage_pasteObjects( origin.manage_cutObjects(o_id) )

    new = getattr(weblog, o_id)
    raise info.ObjectMoved(new, new)


def moveToArchive(self, info):
    o = info.object
    q_tool = getToolByName(self, 'quills_tool')

    # Set the creatorFullname for pretty display
    member = getToolByName(self, 'portal_membership').getAuthenticatedMember()
    fullname = member.getProperty('fullname')
    o.setCreatorFullname(fullname)
    weblog = q_tool.getParentWeblog(self)
    origin = o.aq_parent
    archive = getattr(weblog, 'archive')

    dest = archive.createPath(o.EffectiveDate())

    o_id = o.getId()
    dest.manage_pasteObjects( origin.manage_cutObjects(o_id) )

    new = getattr(dest, o_id)

    # Send trackback ping if it's there
    if new.getTrackbackURL():
        q_tool.sendTrackbackPing(new)

    raise info.ObjectMoved(new, new)

