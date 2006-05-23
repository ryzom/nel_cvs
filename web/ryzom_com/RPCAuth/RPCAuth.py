"""
RPCAuth is a pretraversal hook that checks for xmlrpc calls and
tries to extract a username and password from the arguements passed
"""

import sys
from base64 import encodestring
from urllib import quote, unquote
from os import path
import string
from DateTime import DateTime
from utils import SimpleItemWithProperties
from AccessControl import ClassSecurityInfo, Permissions
from ZPublisher import BeforeTraverse
import Globals
from zLOG import LOG, ERROR
from App.Common import package_home
from ZPublisher.HTTPRequest import HTTPRequest

# Constants.
ATTEMPT_DISABLED = -1
ATTEMPT_NONE = 0
ATTEMPT_LOGIN = 1
ATTEMPT_CONT = 2

ModifyRPCAuth = 'Change RPC Auth'


class RPCAuth(SimpleItemWithProperties):
    '''
    Reads xmlrpc args during traversal and simulates the HTTP
    authentication headers.
    '''
    meta_type = 'RPC Auth'
    security = ClassSecurityInfo()
    security.declareProtected(ModifyRPCAuth,
                              'manage_editProperties',
                              'manage_changeProperties')
    security.declareProtected(Permissions.view_management_screens,
                              'manage_propertiesForm')


    _properties = ({'id':'uname_arg', 'type': 'string', 'mode':'w',
                    'label':'User name arguement prefix'},
                   {'id':'pword_arg', 'type': 'string', 'mode':'w',
                    'label':'User password arguement prefix'},
                   {'id':'remove_args','type':'boolean','mode':'w',
                    'label':'Remove password username arguements'},
                   )

    def __init__(self):
        self.remove_args = 1
        self.uname_arg = 'zid'
        self.pword_arg = 'zpw'
        self._authProviders = {'absglob':{},'abs':{},'rel':{}}


    security.declareProtected(ModifyRPCAuth,'addAuthProvider')
    def addAuthProvider(self,objectPaths,function):
        """takes a list of paths and a function as an arguements"""
        aps = getattr(self,'_authProviders',{'absglob':{},'abs':{},'rel':{}})
        for name in list(objectPaths):
            if name[-1] == '/' and name[0] == '/': # absolute glob
                aps['absglob'][tuple(string.split(name,'/')[1:-1])] = function
            if name[0] == '/':  # absolute url
                aps['abs'][tuple(string.split(name,'/')[1:])] = function
            if name[0] != '/' and name[-1] != '/' and '/' in name: # relative
                aps['rel'][tuple(string.split(name,'/'))] = function
        self._authProviders = aps
        self._p_changed = 1

    security.declareProtected(ModifyRPCAuth,'listAuthProviders')
    def listAuthProviders(self):
        """a list of objects with auth providers"""
        aps = getattr(self,'_authProviders',{'absglob':{},'abs':{},'rel':{}})
        return aps['absglob'].keys()+\
               aps['abs'].keys()+\
               aps['rel'].keys()

    security.declareProtected(ModifyRPCAuth,'removeAuthProvider')
    def removeAuthProvider(self,objectPaths):
        """remove auth providers objectPaths should be a list"""
        for objectPath in list(objectPaths):
            if type(objectPath) == type('s'):
                objectPath = string.split(objectPath,'/')
                while '' in objectPath: del objectPath[objectPath.index('')]
                tuple(objectPath)
            if objectPath in self._authProviders['absglob'].keys():
                del self._authProviders['absglob'][objectPath]
            elif objectPath in self._authProviders['abs'].keys():
                del self._authProviders['abs'][objectPath]
            elif objectPath in self._authProviders['rel'].keys():
                del self._authProviders['rel'][objectPath]


    def _identify(self,arg_tuple):
        arg_list = list(arg_tuple)
        zusername,zpassword = None,None
        try:
            for item in arg_tuple: #use the tuple here so we can delete items as we iterate
                if type(item) == type('') and len(item) > len(self.uname_arg) and item[:len(self.uname_arg)] == self.uname_arg:
                    zusername = item[len(self.uname_arg):]
                    del arg_list[arg_list.index(item)]
                    continue
                if type(item) == type('') and len(item) > len(self.uname_arg) and item[:len(self.pword_arg)] == self.pword_arg:
                    zpassword = item[len(self.pword_arg):]
                    del arg_list[arg_list.index(item)]
            if zusername and zpassword:
                arg_tuple = tuple(arg_list)
                return zusername,zpassword,arg_tuple
            else:
                return None,None,None
        except 'none':
            return None

    def _registryIdentify(self,targetPath):
        targetPath = list(targetPath)
        targetPath.reverse()
        targetPath = [a.encode('utf-8') for a in targetPath]
        targetPath = tuple(targetPath)
        # first check absolute path cuz it's easy
        if targetPath in self._authProviders['abs'].keys():
            return self._authProviders['abs'][targetPath]
        # check relative paths
        relPath = targetPath[:]
        while len(relPath) > 1:
            if relPath in self._authProviders['rel'].keys():
                return self._authProviders['rel'][relPath]
            relPath= relPath[1:]
        # check absolute globs
        globPath = targetPath[:-1]
        while len(globPath) > 1:
            if globPath in self._authProviders['absglob'].keys():
                return self._authProviders['absglob'][globPath]
            globPath=globPath[:-1]
        return None

    security.declarePrivate('modifyRequest')
    def modifyRequest(self, req, resp):
        # Returns flags indicating what the user is trying to do.

        if req.__class__ is not HTTPRequest:
            return ATTEMPT_DISABLED

        if not req._auth:
                # Attempt to log in.
            targetPath = tuple(req['TraversalRequestNameStack'])
            authProvider = self._registryIdentify(targetPath)
            if authProvider:
                name,pw,arg_tuple = authProvider(req.args)
            else:
                name,pw,arg_tuple = self._identify(req.args)
            if name and pw:
                ac = encodestring('%s:%s' % (name, pw))
                req._auth = 'basic %s' % ac
                resp._auth = 1
                if self.remove_args or authProvider: # if we set remove_args or an authProvider is used
                    req.args = arg_tuple
                return ATTEMPT_LOGIN
        return ATTEMPT_NONE

    def __call__(self, container, req):
        '''The __before_publishing_traverse__ hook.'''
        resp = self.REQUEST['RESPONSE']
        attempt = self.modifyRequest(req, resp)


    def _cleanupResponse(self):
        resp = self.REQUEST['RESPONSE']
        try: del resp.unauthorized
        except: pass
        try: del resp._unauthorized
        except: pass
        return resp



    security.declarePrivate('unauthorized')
    def unauthorized(self):
        resp.unauthorized()

    def _unauthorized(self):
        resp._unauthorized()


    # Installation and removal of traversal hooks.

    def manage_beforeDelete(self, item, container):
        if item is self:
            handle = self.meta_type + '/' + self.getId()
            BeforeTraverse.unregisterBeforeTraverse(container, handle)

    def manage_afterAdd(self, item, container):
        if item is self:
            handle = self.meta_type + '/' + self.getId()
            container = container.this()
            nc = BeforeTraverse.NameCaller(self.getId())
            BeforeTraverse.registerBeforeTraverse(container, nc, handle)

Globals.InitializeClass(RPCAuth)

def manage_addRAForm(self):
    "this is a form to get the id"
    return """<html>
    <head>
       <title>Setup RPC Auth</title>
    </head>
    <body>
    Please type the id of the RPC Auth:<br>
        <form name="form" action="manage_addRPCAuth"><br>
        <input type="text" name="id"><br>
        <input type="submit" value="add">
        </form>
    </body>
    </html>"""

def manage_addRPCAuth(self, id, REQUEST=None):
    ' '
    ob = RPCAuth()
    ob.id = id
    self._setObject(id, ob)
    if REQUEST is not None:
        return self.manage_main(self, REQUEST)
