package org.jpy;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * @author Norman Fomferra
 */
@Target(value = ElementType.PARAMETER)
@Retention(value = RetentionPolicy.RUNTIME)
public @interface Mutable {
}
