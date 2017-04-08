package com.me.fastsocks.utils;

import com.me.fastsocks.base.DispatchQueue;

/**
 * Created by zaki on 17/3/26.
 */

public class BaseUtils {
    public static volatile DispatchQueue nativeQueue = new DispatchQueue("nativeQueue");
}
