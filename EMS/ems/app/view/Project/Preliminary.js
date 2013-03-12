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
Ext.require([
                'Ext.grid.*',
                'Ext.data.*',
                'Ext.dd.*'
            ]);

Ext.define('EMS.view.Project.Preliminary', {
               extend: 'Ext.window.Window',
               requires: ['Ext.form.Panel'],
               title : 'Add Preliminary Results',
               id: 'ProjectPreliminary',
               layout: 'fit',
               iconCls: 'default',
               //closeAction: 'hide',
               maximizable: true,
               collapsible: true,
               constrain: true,
               minHeight: 550,
               minWidth: 700,
               height: 550,
               width: 900,
               initComponent: function() {
                   var me = this;
                   var closebutt={
                       minWidth: 90,
                       text: 'Close',
                       handler: function() { me.close(); }
                   };
                   me.dockedItems= [{
                                        xtype: 'toolbar',
                                        dock: 'bottom',
                                        ui: 'footer',
                                        layout: {
                                            pack: 'center'
                                        },
                                        items: [closebutt]
                                    }];

                   me.labDataStore=EMS.store.LabData;
                   me.m_PagingToolbar = Ext.create('Ext.PagingToolbar', {
                                                       store: me.labDataStore,
                                                       displayInfo: true
                                                   });

                   me.items=[ {
                                 xtype: 'container',
                                 layout: {
                                     type: 'hbox',
                                     align: 'stretch'
                                 },
                                 items: [
                                     {
                                         xtype: 'container',
                                         margin: '0 5 5 5',
                                         layout: {
                                             type: 'vbox',
                                             align: 'stretch'
                                         },
                                         flex: 1,
                                         items: [
                                             {
                                                 xtype: 'fieldset',
                                                 title: 'Grouping Results',
                                                 layout: {
                                                     type: 'hbox'
                                                 },
                                                 items: [{
                                                         xtype: 'textfield',
                                                         id: 'preliminary-group-name',
                                                         fieldLabel: 'Group name',
                                                         submitValue: false,
                                                         labelAlign: 'top',
                                                         labelWidth: 120
                                                     } , {
                                                         xtype: 'button',
                                                         margin: '20 5 0 5',
                                                         width: 90,
                                                         text: 'add',
                                                         id: 'preliminary-group-add',
                                                         submitValue: false,
                                                         iconCls: 'folder-add'
                                                     }]
                                             } , {
                                                 xtype: 'treepanel',
                                                 useArrows: true,
                                                 store: me.resultStore,
                                                 rootVisible: false,
                                                 //singleExpand: false,
                                                 border: true,
                                                 columnLines: true,
                                                 rowLines: true,
                                                 flex: 1,
                                                 columns: [{
                                                         xtype: 'treecolumn',
                                                         text: 'Groups/items',
                                                         flex: 1,
                                                         width: 70,
                                                         dataIndex: 'item'
                                                     },{ header: 'Description', dataIndex: 'description', flex:2,
                                                         renderer: function(value,meta,record) {
                                                             if(record.data.leaf===false) return '';
                                                             return record.data['description'];
                                                         }
                                                     },{
                                                         xtype: 'actioncolumn',
                                                         width:35,
                                                         align: 'center',
                                                         sortable: false,
                                                         items: [{
                                                                 getClass: function(v, meta, rec) {
                                                                     if(rec.data.root === true)
                                                                         return;
                                                                     this.items[0].tooltip = 'Delete';
                                                                     this.items[0].handler = function(grid, rowIndex, colIndex, actionItem, event, record, row) {
                                                                         try {
                                                                             record.remove(true);
                                                                         } catch (error) {
                                                                             Logger.log("Error:"+error);
                                                                         }
                                                                         grid.getStore().sync();
                                                                     }
                                                                     if(rec.data.leaf===false)
                                                                         return 'folder-delete';
                                                                     return 'table-row-delete';
                                                                 }
                                                             }]
                                                     }

                                                 ],
                                                 viewConfig: {
                                                     copy: true,
                                                     enableTextSelection: false,
                                                     plugins: {
                                                         ptype: 'treeviewdragdrop',
                                                         appendOnly: true,
                                                         dropGroup: 'ldata2results'
                                                     },
                                                     listeners: {
                                                         beforedrop: function(node,data,overModel,dropPosition,dropFunction,eOpts) {
                                                             //Logger.log(data);
                                                             //Logger.log(overModel);
                                                             //Logger.log(dropPosition);
                                                             if(overModel.data.root === true)
                                                                 return false;
                                                             if(dropPosition !== 'append' && overModel.data.leaf === false)
                                                                 return false;
                                                             var base=overModel.childNodes.length;
                                                             for(var i=0; i<data.records.length;i++) {
                                                                 var cont=false;
                                                                 for(var j=0; j<overModel.childNodes.length;j++) {
                                                                     //Logger.log(data.records[i].data.id);
                                                                     //Logger.log(overModel.childNodes[j]);
                                                                     if(data.records[i].data.id===overModel.childNodes[j].data.labdata_id) {
                                                                         data.records.splice(i,1);
                                                                         cont=true;
                                                                         i--;
                                                                         break;
                                                                         //return false;
                                                                     }
                                                                 }
                                                                 if(cont) continue;

                                                                 data.records[i].set('leaf', true);
                                                                 data.records[i].set('item', overModel.data.item+' '+(base+i+1));
                                                                 data.records[i].set('item_id',data.records[i].data.id);
                                                                 data.records[i].set('project_id',me.project_id);
                                                                 data.records[i].set('labdata_id',data.records[i].data.id);
                                                                 data.records[i].set('rtype_id',Ext.getCmp('preliminary-type-changed').getValue());
                                                                 data.records[i].set('description', Ext.String.format('<b>id: <i>{2}</i>&nbsp;date: <i>{1}</i></b>&nbsp;<small>[ {0};{3} ]</small><br><small>{4}</small>',
                                                                                                                      data.records[i].data.cells,
                                                                                                                      Ext.util.Format.date(data.records[i].data.dateadd,'m/d/Y'),
                                                                                                                      data.records[i].data.id, data.records[i].data.conditions,data.records[i].data.name4browser));
                                                             }
                                                             return true;
                                                         },
                                                         drop: function(node,data,overModel,dropPosition,dropFunction,eOpts) {
                                                             me.resultStore.sync();
                                                             me.resultStore.load();
                                                         }
                                                     }
                                                 }
                                             }//treepanel
                                         ]
                                     } , {
                                         xtype: 'container',
                                         margin: '0 5 5 0',
                                         layout: {
                                             type: 'vbox',
                                             align: 'stretch'
                                         },
                                         flex: 1,
                                         items: [
                                             {
                                                 xtype: 'fieldset',
                                                 title: 'Filtering Results',
                                                 layout: {
                                                     type: 'hbox',
                                                     align: 'stretch'
                                                 },
                                                 //height: 80,
                                                 items: [{
                                                         xtype: 'combobox',
                                                         id: 'preliminary-worker-changed',
                                                         displayField: 'fullname',
                                                         editable: false,
                                                         valueField: 'id',
                                                         flex: 4,
                                                         fieldLabel: 'Worker',
                                                         labelAlign: 'top',
                                                         labelWidth: 120,
                                                         store: EMS.store.Worker,
                                                         value: USER_ID
                                                     } , {
                                                         xtype: 'combobox',
                                                         displayField: 'name',
                                                         valueField: 'id',
                                                         editable: false,
                                                         id: 'preliminary-type-changed',
                                                         value: 1,
                                                         fieldLabel: 'Type',
                                                         labelAlign: 'top',
                                                         labelWidth: 120,
                                                         margin: '0 5 0 5',
                                                         store: EMS.store.RType,
                                                         flex: 4
                                                     }]
                                             } , {
                                                 xtype: 'grid',
                                                 border: true,
                                                 columnLines: true,
                                                 store: me.labDataStore,
                                                 multiSelect: true,
                                                 viewConfig: {
                                                     plugins: {
                                                         ptype: 'gridviewdragdrop',
                                                         dragGroup: 'ldata2results',
                                                     },
                                                     listeners: {
                                                         drop: function(node, data, dropRec, dropPosition) {
                                                         }
                                                     },
                                                     copy: true,
                                                     enableTextSelection: false
                                                 },
                                                 columns: [
                                                     { header: 'Experiment list', dataIndex: 'cells', flex:1, sortable: true,
                                                         renderer: function(value,meta,record) {
                                                             return Ext.String.format('<b>id: <i>{2}</i>&nbsp;date: <i>{1}</i></b>&nbsp;<small> {0} </small><br>{3}<br>{4}',
                                                                                      record.data['cells'],
                                                                                      Ext.util.Format.date(record.data['dateadd'],'m/d/Y'),
                                                                                      record.data['id'], record.data['conditions'],record.data['name4browser']);
                                                         }
                                                     }
                                                 ],
                                                 flex: 3,
                                                 bbar: [ me.m_PagingToolbar ]
                                             }
                                         ]
                                     }
                                 ]
                             }
                           ];


                   me.callParent(arguments);
               }
           });