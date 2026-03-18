export const getKeyByValue = (object: any, value: any) => {
  if (!object) {
    return null
  }
  return Object.keys(object).find((key) => object[key] === value)
}

export const getKeyByValueExcluding = (object: any, key: string, value: any) => {
  if (!object) {
    return null
  }
  return Object.keys(object)
    .filter((k) => k !== key)
    .find((key) => object[key] === value)
}

export const getTrueKeys = (object: any) => {
  if (!object) {
    return []
  }

  const trueKeys: any = []
  for (const [key, value] of Object.entries(object)) {
    if (object[key]) {
      trueKeys.push(key)
    }
  }
  return trueKeys
}
