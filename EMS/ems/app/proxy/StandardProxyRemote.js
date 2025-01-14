/****************************************************************************
 **
 ** Copyright (C) 2011 Andrey Kartashov .
 ** All rights reserved.
 ** Contact: Andrey Kartashov (porter@porter.st)
 **
 ** This file is part of the EMS web interface module of the genome-tools.
 **
 ** GNU Lesser General Public License Usage
 ** This file may be used under the terms of the GNU Lesser General Public
 ** License version 2.1 as published by the Free Software Foundation and
 ** appearing in the file LICENSE.LGPL included in the packaging of this
 ** file. Please review the following information to ensure the GNU Lesser
 ** General Public License version 2.1 requirements will be met:
 ** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** Other Usage
 ** Alternatively, this file may be used in accordance with the terms and
 ** conditions contained in a signed written agreement between you and Andrey Kartashov.
 **
 ****************************************************************************/

Ext.define('EMS.proxy.StandardProxyRemote', {
    extend: 'Ext.data.proxy.Ajax',
    alias: 'proxy.standardproxyremote',
    timeout: 600000,
    api: {
        read: 'data/GeneralList.php',
        update: 'data/GeneralListUp.php',
        create: 'data/GeneralListAdd.php',
        destroy: 'data/GeneralListDel.php'
    },
    showMessage: true,
//    extraParams: {
//        //tablename:  ''
//    },
    reader: {
        type: 'json',
        root: 'data',
        successProperty: 'success'
    },
    writer: {
        type: 'json',
        root: 'data',
        writeAllFields: true,
        encode: true
    },
    listeners: {
        exception: function (proxy, response, operation) {
            try {
                var json = Ext.decode(response.responseText);
                if (json && !json.success != "success") {
                    if(this.showMessage)
                    Ext.MessageBox.show({
                                            title: operation.action + ' failed',
                                            msg: json.message,
                                            icon: Ext.MessageBox.ERROR,
                                            buttons: Ext.Msg.OK
                                        });
                    EMS.util.Util.Logger.log(operation + ' failedm:' + json.message);
                    //console.log(operation, ' failedm:', json.message, ' data:', json);
                } else {
                    if(this.showMessage)
                        Ext.MessageBox.show({
                                            title: operation.action + ' failed',
                                            msg: operation.getError(),
                                            icon: Ext.MessageBox.ERROR,
                                            buttons: Ext.Msg.OK
                                        });
                }
            }
            catch (Error) {
                Ext.Msg.show({
                                 title: operation.action + ' failed',
                                 msg: 'Error in "' + operation.action + '" operation',
                                 icon: Ext.Msg.ERROR,
                                 buttons: Ext.Msg.OK
                             });
            }
        }
    } //listeners
});
