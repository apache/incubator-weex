const lib = window.lib || (window.lib = {})

/**
 * Version class.
 * @class lib.env~Version
 * @param {String} v - version number.
 */
export default function Version (v) {
  Object.defineProperty(this, 'val', {
    value: v.toString(),
    enumerable: true
  })

  /**
   * larger than
   * @method gt
   * @param {String} v - version
   * @return {Boolean} result
   * @instance
   * @memberof Version
   */
  this.gt = function (v) {
    return Version.compare(this, v) > 0
  }

  /**
   * larger than or equal to.
   * @method gte
   * @param {String} v - version
   * @return {Boolean} result
   * @instance
   * @memberof Version
   */
  this.gte = function (v) {
    return Version.compare(this, v) >= 0
  }

  /**
   * less than.
   * @method lt
   * @param {String} v - version
   * @return {Boolean} result
   * @instance
   * @memberof Version
   */
  this.lt = function (v) {
    return Version.compare(this, v) < 0
  }

  /**
   * less than or equal to.
   * @method lte
   * @param {String} v - version
   * @return {Boolean} result
   * @instance
   * @memberof Version
   */
  this.lte = function (v) {
    return Version.compare(this, v) <= 0
  }

  /**
   * equal to.
   * @method eq
   * @param {String} v - version
   * @return {Boolean} equal to
   * @instance
   * @memberof Version
   */
  this.eq = function (v) {
    return Version.compare(this, v) === 0
  }
}

/**
 * version number string.
 * @method toString
 * @return {String} current version number string.
 * @instance
 * @memberof Version
 */
Version.prototype.toString = function () {
  return this.val
}

/**
 * return current version number.
 * @method valueOf
 * @return {Boolean} version number
 * @instance
 * @memberof Version
 */
Version.prototype.valueOf = function () {
  const v = this.val.split('.')
  const r = []
  for (let i = 0; i < v.length; i++) {
    let n = parseInt(v[i], 10)
    if (isNaN(n)) {
      n = 0
    }
    let s = n.toString()
    if (s.length < 5) {
      s = Array(6 - s.length).join('0') + s
    }
    r.push(s)
    if (r.length === 1) {
      r.push('.')
    }
  }
  return parseFloat(r.join(''))
}

/**
 * compare two versions.
 * @method compare
 * @param {String} v1 - version1
 * @param {String} v2 - version2
 * @return {Number} 0 for equality，-1 for less than，1 for larger than.
 * @memberof Version
 */
Version.compare = function (v1, v2) {
  v1 = v1.toString().split('.')
  v2 = v2.toString().split('.')
  for (let i = 0; i < v1.length || i < v2.length; i++) {
    let n1 = parseInt(v1[i], 10)
    let n2 = parseInt(v2[i], 10)
    if (window.isNaN(n1)) {
      n1 = 0
    }
    if (window.isNaN(n2)) {
      n2 = 0
    }
    if (n1 < n2) {
      return -1
    }
    else if (n1 > n2) {
      return 1
    }
  }
  return 0
}

/**
 * 解析和操作版本号
 * @method version
 * @param {string} v - 需要解析的版本号
 * @return {lib.env~Version} Verson实例
 * @memberof lib
 */
lib.version = function (v) {
  return new Version(v)
}
