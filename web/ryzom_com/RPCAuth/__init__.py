import RPCAuth

def initialize(context):
    context.registerClass(
        RPCAuth.RPCAuth,
        constructors=(RPCAuth.manage_addRAForm,
                      RPCAuth.manage_addRPCAuth),
        icon = 'RPCAuth.gif'
        )
