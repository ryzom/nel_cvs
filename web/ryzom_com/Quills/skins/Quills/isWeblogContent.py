## Script (Python) "sWeblogContent"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##parameters=
##title=Return true if we're in weblog content types
##


if context.meta_type in ('Weblog', 'WeblogArchive',
                         'WeblogEntry', 'WeblogTopic'):
    return True
else:
    return False
