About
=====

Quills is an Enterprise Weblog System for the Plone content management system.
It is designed from the ground up to work well and provide specialized features
for a multi-blog, multi-user environment.


Tips
====

Weblog as the front page
------------------------

There are a number of ways to get your weblog to show up as the front page of
your website:

1) Somewhat easy, good URL's (recommended):
    Give your weblog an id like 'weblog' or 'blog'.  Then open up the ZMI view
    of your Plone site and goto portal_properties/site_properties.  Add
    'weblog' or 'blog' (whatever id you gave to your weblog, it can really be
    anything) to the bottom of the 'default_page' properties.  Now when you
    have an object with the id of 'weblog' or 'blog' (whichever you added) it
    can be used as the index.  Remember to remove/rename any other objects in
    the folder with id's like index_html or index.html, because if they are
    there then they will be used as the index instead of your weblog.

    This will give URL's like:
        http://example.com/weblog/archive/2004/08/09/my_entry

    This is a little better since the URL now holds move obvious meaning to
    the human reader.  

2) Easier, but somewhat ugly URL:
    Give your weblog object an id that matches one of the default 'index page'
    id's in your plone site.  Usually this is either 'index_html' or
    'index.html'.  The downside here is that your weblog entries will have
    URL's like:
        http://example.com/index_html/archive/2004/08/09/my_entry

2) Hard way, has issues, not recommended, requires Apache proxy setup:

    If you are using Apache to proxy your plone instance, you can simply add
    the name of the weblog object to the path you give to ProxyPass:

        <VirtualHost *:80>
          Servername <servername>
          ProxyPass / http://localhost:8080/VirtualHostBase/http/<servername>:80/<plone-instance>/<weblog-object>/VirtualHostRoot/
        </VirtualHost>

    This method can have confusing effects on your website.  Use at your own
    risk.


Resources
=========

    Development Homepage: http://developer.etria.com/projects/quills/
    Mailing List: http://lists.etria.com/cgi-bin/mailman/listinfo/quills-dev


Other Zope/Plone Weblog Products
================================

Plone Compatible:
    CMFWeblog   http://www.sf.net/projects/collective/
    SimpleBlog  http://www.sf.net/projects/collective/

Zope Only:
    COREBlog    http://coreblog.org/

