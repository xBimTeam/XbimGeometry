using System.Collections.Generic;
using Xbim.Common;

namespace Xbim.Geometry.Abstractions.Extensions
{
    public static class IXModelExtensions
    {
        /// <summary>
        /// Treat the tag property of IModel a dictionary of key value pairs
        /// allowing more than one object to be stored against the dictionary
        /// NB this can be overriden if another user sets the Tag property directly
        /// It is only safe in scenarios where model ownership is well defined
        /// </summary>
        /// <param name="model"></param>
        /// <param name="key"></param>
        /// <param name="value"></param>
        /// <returns>false if another user has set the tage property or the key is already in use</returns>
        public static bool AddTagValue(this IModel model, string key, object value)
        {
            if (model.Tag is null)
            {
                var keyValuePairs = new Dictionary<string, object>();
                keyValuePairs.Add(key, value);
                model.Tag = keyValuePairs;
                return true;
            }
            else if (model.Tag is IDictionary<string, object> keyValuePairs)
            {
                if (!keyValuePairs.ContainsKey(key))
                {
                    keyValuePairs.Add(key, value);
                    return true;
                }
            }
            return false;
        }
        /// <summary>
        /// Sets a value with a key against the tag property
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="model"></param>
        /// <param name="key"></param>
        /// <param name="value"></param>
        /// <returns>returnd false if the tag property is not initialised as a KeyValue pair dictionary or the key has not been set with a value</returns>
        public static bool GetTagValue<T>(this IModel model, string key, out T? value)
        {
            if (model.Tag is not null && model.Tag is IDictionary<string, object> keyValuePairs)
            {
                if (keyValuePairs.TryGetValue(key, out object val))
                {
                    if (val is T)
                    {
                        value = (T)val;
                        return true;
                    }
                }
            }
            value = default;
            return false;
        }
        /// <summary>
        /// Removes a keyed value from the Tag property disctionary
        /// </summary>
        /// <param name="model"></param>
        /// <param name="key"></param>
        /// <returns>returns false if the dictionary has not been initialised or the key is not present in the dictionary</returns>
        public static bool RemoveTagValue(this IModel model, string key)
        {
            if (model.Tag is not null && model.Tag is IDictionary<string, object> keyValuePairs)
                return keyValuePairs.Remove(key);
            else
                return false;
        }
    }
}
