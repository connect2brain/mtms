//import { css as cssFn, ThemedCssFunction } from 'styled-components';

//const css = cssFn as unknown as ThemedCssFunction<never>;

import styled from 'styled-components'

const pxToRem = (px: number) => `${px / 16}rem`

const theme = {
  rem: pxToRem,
  colors: {
    red: 'red',
    primary: 'black',
    disabled: 'grey',
    error: '#a21010',
    ok: '#258f25',
    white: '#FFFFFF',
    lightergray: '#f1f1f1',
    lightgray: '#e0e0e0',
    gray: '#b0b0b0',
    darkgray: '#707070',
  },
  spacing: {},
  typography: {
    large: pxToRem(24),
    medium: pxToRem(18),
    small: pxToRem(12),
  },
  borderRadius: pxToRem(5),
}

export type Theme = typeof theme

export type Color = keyof Theme['colors']
export type Spacing = keyof Theme['spacing']

export default theme
