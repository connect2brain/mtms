import React from 'react'
import ReactDOM from 'react-dom/client'
import './index.css'
import App from './App'
import { BrowserRouter } from 'react-router-dom'
import { Provider } from 'react-redux'
import { store } from './providers/store'

const ensureDebugOverlay = () => {
  let el = document.getElementById('mtms-debug-overlay')
  if (!el) {
    el = document.createElement('pre')
    el.id = 'mtms-debug-overlay'
    el.style.position = 'fixed'
    el.style.top = '0'
    el.style.left = '0'
    el.style.right = '0'
    el.style.maxHeight = '45vh'
    el.style.overflow = 'auto'
    el.style.margin = '0'
    el.style.padding = '12px'
    el.style.whiteSpace = 'pre-wrap'
    el.style.zIndex = '999999'
    el.style.background = 'rgba(120, 0, 0, 0.95)'
    el.style.color = '#fff'
    el.style.fontSize = '12px'
    el.style.fontFamily = 'monospace'
    el.style.borderBottom = '1px solid #300'
    document.body.appendChild(el)
  }
  return el
}

const showDebugOverlay = (title: string, details: string) => {
  const el = ensureDebugOverlay()
  el.textContent = `${title}\n\n${details}`
}

window.addEventListener('error', (event) => {
  const details = event.error?.stack || event.message || 'Unknown window error'
  showDebugOverlay('Runtime error', String(details))
})

window.addEventListener('unhandledrejection', (event) => {
  const reason = event.reason
  const details =
    reason?.stack ||
    (typeof reason === 'string' ? reason : JSON.stringify(reason, null, 2)) ||
    'Unknown unhandled rejection'
  showDebugOverlay('Unhandled promise rejection', String(details))
})

const root = ReactDOM.createRoot(document.getElementById('root') as HTMLElement)
root.render(
  <Provider store={store}>
    <BrowserRouter>
      <App />
    </BrowserRouter>
  </Provider>
)
