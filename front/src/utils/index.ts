export const expand = (obj: any) =>
  obj
    ? Object.keys(obj)
        .map((key) => obj[key].toFixed(3))
        .join(', ')
    : ''

const isObject = function (obj: any) {
  return obj === Object(obj) && !Array.isArray(obj) && typeof obj !== 'function'
}
const camelToUnderscore = (key: string) => {
  return key.replace(/([A-Z])/g, '_$1').toLowerCase()
}
const snakeToCamel = (str: string) => {
  return str.replace(/([-_][a-z])/gi, ($1) => {
    return $1.toUpperCase().replace('-', '').replace('_', '')
  })
}

export const objectKeysToSnakeCase = (obj: any): any => {
  if (isObject(obj)) {
    const n: any = {}

    Object.keys(obj).forEach((k) => {
      n[camelToUnderscore(k)] = objectKeysToSnakeCase(obj[k])
    })

    return n
  } else if (Array.isArray(obj)) {
    return obj.map((i) => {
      return objectKeysToSnakeCase(i)
    })
  }

  return obj
}
