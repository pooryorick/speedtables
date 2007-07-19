# Tcl package index file, version 1.1
# This file is generated by the "pkg_mkIndex" command
# and sourced either when an application starts up or
# by a "package unknown" script.  It invokes the
# "package ifneeded" command to set up package-related
# information so that packages will be loaded automatically
# in response to "package require" commands.  When this
# script is sourced, the variable $dir must contain the
# full path name of this file's directory.

package ifneeded st_client 1.0 [list source [file join $dir client/client.tcl]]
package ifneeded st_client_postgres 1.0 [list source [file join $dir client/pgsql.tcl]]
package ifneeded st_debug 1.0 [list source [file join $dir debug.tcl]]
package ifneeded st_display 1.0 [list source [file join $dir display/display.tcl]]
package ifneeded st_display_test 1.0 [list source [file join $dir display/test.tcl]]
package ifneeded st_locks 1.0 [list source [file join $dir server/lock.tcl]]
package ifneeded st_optimizer 1.0 [list source [file join $dir client/optimizer.tcl]]
package ifneeded st_postgres 1.0 [list source [file join $dir pgsql.tcl]]\n[list source [file join $dir copy.tcl]]
package ifneeded st_server 1.0 [list source [file join $dir server/server.tcl]]
package ifneeded stapi 1.0 [list source [file join $dir stapi.tcl]]
package ifneeded stapi_extend 1.0 [list source [file join $dir client/extend.tcl]]
