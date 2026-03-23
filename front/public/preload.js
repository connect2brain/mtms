const { contextBridge, ipcRenderer } = require('electron')

contextBridge.exposeInMainWorld('electronAPI', {
  saveFile: (defaultName, content) => ipcRenderer.invoke('dialog:saveFile', defaultName, content),
  loadFile: () => ipcRenderer.invoke('dialog:loadFile'),
})
