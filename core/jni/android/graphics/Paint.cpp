/* libs/android_runtime/android/graphics/Paint.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "Paint"

#include <utils/Log.h>

#include "jni.h"
#include "GraphicsJNI.h"
#include <android_runtime/AndroidRuntime.h>
#include <ScopedUtfChars.h>

#include "SkBlurDrawLooper.h"
#include "SkColorFilter.h"
#include "SkMaskFilter.h"
#include "SkRasterizer.h"
#include "SkShader.h"
#include "SkTypeface.h"
#include "SkXfermode.h"
#include "unicode/uloc.h"
#include "unicode/ushape.h"
#include "TextLayout.h"

// temporary for debugging
#include <Caches.h>
#include <utils/Log.h>

namespace android {

struct JMetricsID {
    jfieldID    top;
    jfieldID    ascent;
    jfieldID    descent;
    jfieldID    bottom;
    jfieldID    leading;
};

static jclass   gFontMetrics_class;
static JMetricsID gFontMetrics_fieldID;

static jclass   gFontMetricsInt_class;
static JMetricsID gFontMetricsInt_fieldID;

static void defaultSettingsForAndroid(SkPaint* paint) {
    // GlyphID encoding is required because we are using Harfbuzz shaping
    paint->setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    SkPaintOptionsAndroid paintOpts = paint->getPaintOptionsAndroid();
    paintOpts.setUseFontFallbacks(true);
    paint->setPaintOptionsAndroid(paintOpts);
}

class SkPaintGlue {
public:
    enum MoveOpt {
        AFTER, AT_OR_AFTER, BEFORE, AT_OR_BEFORE, AT
    };

    static void finalizer(JNIEnv* env, jobject clazz, jlong objHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        delete obj;
    }

    static jlong init(JNIEnv* env, jobject clazz) {
        SkPaint* obj = new SkPaint();
        defaultSettingsForAndroid(obj);
        return reinterpret_cast<jlong>(obj);
    }

    static jlong initWithPaint(JNIEnv* env, jobject clazz, jlong paintHandle) {
        SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        SkPaint* obj = new SkPaint(*paint);
        return reinterpret_cast<jlong>(obj);
    }

    static void reset(JNIEnv* env, jobject clazz, jlong objHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        obj->reset();
        defaultSettingsForAndroid(obj);
    }

    static void assign(JNIEnv* env, jobject clazz, jlong dstPaintHandle, jlong srcPaintHandle) {
        SkPaint* dst = reinterpret_cast<SkPaint*>(dstPaintHandle);
        const SkPaint* src = reinterpret_cast<SkPaint*>(srcPaintHandle);
        *dst = *src;
    }

    static jint getFlags(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        int result;
        result = GraphicsJNI::getNativePaint(env, paint)->getFlags();
        return static_cast<jint>(result);
    }

    static void setFlags(JNIEnv* env, jobject paint, jint flags) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setFlags(flags);
    }

    static jint getHinting(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        return GraphicsJNI::getNativePaint(env, paint)->getHinting()
                == SkPaint::kNo_Hinting ? 0 : 1;
    }

    static void setHinting(JNIEnv* env, jobject paint, jint mode) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setHinting(
                mode == 0 ? SkPaint::kNo_Hinting : SkPaint::kNormal_Hinting);
    }

    static void setAntiAlias(JNIEnv* env, jobject paint, jboolean aa) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setAntiAlias(aa);
    }

    static void setLinearText(JNIEnv* env, jobject paint, jboolean linearText) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setLinearText(linearText);
    }

    static void setSubpixelText(JNIEnv* env, jobject paint, jboolean subpixelText) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setSubpixelText(subpixelText);
    }

    static void setUnderlineText(JNIEnv* env, jobject paint, jboolean underlineText) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setUnderlineText(underlineText);
    }

    static void setStrikeThruText(JNIEnv* env, jobject paint, jboolean strikeThruText) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setStrikeThruText(strikeThruText);
    }

    static void setFakeBoldText(JNIEnv* env, jobject paint, jboolean fakeBoldText) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setFakeBoldText(fakeBoldText);
    }

    static void setFilterBitmap(JNIEnv* env, jobject paint, jboolean filterBitmap) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setFilterLevel(
                filterBitmap ? SkPaint::kLow_FilterLevel : SkPaint::kNone_FilterLevel);
    }

    static void setDither(JNIEnv* env, jobject paint, jboolean dither) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setDither(dither);
    }

    static jint getStyle(JNIEnv* env, jobject clazz,jlong objHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        return static_cast<jint>(obj->getStyle());
    }

    static void setStyle(JNIEnv* env, jobject clazz, jlong objHandle, jint styleHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkPaint::Style style = static_cast<SkPaint::Style>(styleHandle);
        obj->setStyle(style);
    }

    static jint getColor(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        int color;
        color = GraphicsJNI::getNativePaint(env, paint)->getColor();
        return static_cast<jint>(color);
    }

    static jint getAlpha(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        int alpha;
        alpha = GraphicsJNI::getNativePaint(env, paint)->getAlpha();
        return static_cast<jint>(alpha);
    }

    static void setColor(JNIEnv* env, jobject paint, jint color) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setColor(color);
    }

    static void setAlpha(JNIEnv* env, jobject paint, jint a) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setAlpha(a);
    }

    static jfloat getStrokeWidth(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        return SkScalarToFloat(GraphicsJNI::getNativePaint(env, paint)->getStrokeWidth());
    }

    static void setStrokeWidth(JNIEnv* env, jobject paint, jfloat width) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setStrokeWidth(SkFloatToScalar(width));
    }

    static jfloat getStrokeMiter(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        return SkScalarToFloat(GraphicsJNI::getNativePaint(env, paint)->getStrokeMiter());
    }

    static void setStrokeMiter(JNIEnv* env, jobject paint, jfloat miter) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setStrokeMiter(SkFloatToScalar(miter));
    }

    static jint getStrokeCap(JNIEnv* env, jobject clazz, jlong objHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        return static_cast<jint>(obj->getStrokeCap());
    }

    static void setStrokeCap(JNIEnv* env, jobject clazz, jlong objHandle, jint capHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkPaint::Cap cap = static_cast<SkPaint::Cap>(capHandle);
        obj->setStrokeCap(cap);
    }

    static jint getStrokeJoin(JNIEnv* env, jobject clazz, jlong objHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        return static_cast<jint>(obj->getStrokeJoin());
    }

    static void setStrokeJoin(JNIEnv* env, jobject clazz, jlong objHandle, jint joinHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkPaint::Join join = (SkPaint::Join) joinHandle;
        obj->setStrokeJoin(join);
    }

    static jboolean getFillPath(JNIEnv* env, jobject clazz, jlong objHandle, jlong srcHandle, jlong dstHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkPath* src = reinterpret_cast<SkPath*>(srcHandle);
        SkPath* dst = reinterpret_cast<SkPath*>(dstHandle);
        return obj->getFillPath(*src, dst) ? JNI_TRUE : JNI_FALSE;
    }

    static jlong setShader(JNIEnv* env, jobject clazz, jlong objHandle, jlong shaderHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkShader* shader = reinterpret_cast<SkShader*>(shaderHandle);
        return reinterpret_cast<jlong>(obj->setShader(shader));
    }

    static jlong setColorFilter(JNIEnv* env, jobject clazz, jlong objHandle, jlong filterHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint *>(objHandle);
        SkColorFilter* filter  = reinterpret_cast<SkColorFilter *>(filterHandle);
        return reinterpret_cast<jlong>(obj->setColorFilter(filter));
    }

    static jlong setXfermode(JNIEnv* env, jobject clazz, jlong objHandle, jlong xfermodeHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkXfermode* xfermode = reinterpret_cast<SkXfermode*>(xfermodeHandle);
        return reinterpret_cast<jlong>(obj->setXfermode(xfermode));
    }

    static jlong setPathEffect(JNIEnv* env, jobject clazz, jlong objHandle, jlong effectHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkPathEffect* effect  = reinterpret_cast<SkPathEffect*>(effectHandle);
        return reinterpret_cast<jlong>(obj->setPathEffect(effect));
    }

    static jlong setMaskFilter(JNIEnv* env, jobject clazz, jlong objHandle, jlong maskfilterHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkMaskFilter* maskfilter  = reinterpret_cast<SkMaskFilter*>(maskfilterHandle);
        return reinterpret_cast<jlong>(obj->setMaskFilter(maskfilter));
    }

    static jlong setTypeface(JNIEnv* env, jobject clazz, jlong objHandle, jlong typefaceHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkTypeface* typeface = reinterpret_cast<SkTypeface*>(typefaceHandle);
        return reinterpret_cast<jlong>(obj->setTypeface(typeface));
    }

    static jlong setRasterizer(JNIEnv* env, jobject clazz, jlong objHandle, jlong rasterizerHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkRasterizer* rasterizer = reinterpret_cast<SkRasterizer*>(rasterizerHandle);
        return reinterpret_cast<jlong>(obj->setRasterizer(rasterizer));
    }

    static jint getTextAlign(JNIEnv* env, jobject clazz, jlong objHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        return static_cast<jint>(obj->getTextAlign());
    }

    static void setTextAlign(JNIEnv* env, jobject clazz, jlong objHandle, jint alignHandle) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        SkPaint::Align align = static_cast<SkPaint::Align>(alignHandle);
        obj->setTextAlign(align);
    }

    // generate bcp47 identifier for the supplied locale
    static void toLanguageTag(char* output, size_t outSize,
            const char* locale) {
        if (output == NULL || outSize <= 0) {
            return;
        }
        if (locale == NULL) {
            output[0] = '\0';
            return;
        }
        char canonicalChars[ULOC_FULLNAME_CAPACITY];
        UErrorCode uErr = U_ZERO_ERROR;
        uloc_canonicalize(locale, canonicalChars, ULOC_FULLNAME_CAPACITY,
                &uErr);
        if (U_SUCCESS(uErr)) {
            char likelyChars[ULOC_FULLNAME_CAPACITY];
            uErr = U_ZERO_ERROR;
            uloc_addLikelySubtags(canonicalChars, likelyChars,
                    ULOC_FULLNAME_CAPACITY, &uErr);
            if (U_SUCCESS(uErr)) {
                uErr = U_ZERO_ERROR;
                uloc_toLanguageTag(likelyChars, output, outSize, FALSE, &uErr);
                if (U_SUCCESS(uErr)) {
                    return;
                } else {
                    ALOGD("uloc_toLanguageTag(\"%s\") failed: %s", likelyChars,
                            u_errorName(uErr));
                }
            } else {
                ALOGD("uloc_addLikelySubtags(\"%s\") failed: %s",
                        canonicalChars, u_errorName(uErr));
            }
        } else {
            ALOGD("uloc_canonicalize(\"%s\") failed: %s", locale,
                    u_errorName(uErr));
        }
        // unable to build a proper language identifier
        output[0] = '\0';
    }

    static void setTextLocale(JNIEnv* env, jobject clazz, jlong objHandle, jstring locale) {
        SkPaint* obj = reinterpret_cast<SkPaint*>(objHandle);
        ScopedUtfChars localeChars(env, locale);
        char langTag[ULOC_FULLNAME_CAPACITY];
        toLanguageTag(langTag, ULOC_FULLNAME_CAPACITY, localeChars.c_str());

        SkPaintOptionsAndroid paintOpts = obj->getPaintOptionsAndroid();
        paintOpts.setLanguage(langTag);
        obj->setPaintOptionsAndroid(paintOpts);
    }

    static jfloat getTextSize(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        return SkScalarToFloat(GraphicsJNI::getNativePaint(env, paint)->getTextSize());
    }

    static void setTextSize(JNIEnv* env, jobject paint, jfloat textSize) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setTextSize(SkFloatToScalar(textSize));
    }

    static jfloat getTextScaleX(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        return SkScalarToFloat(GraphicsJNI::getNativePaint(env, paint)->getTextScaleX());
    }

    static void setTextScaleX(JNIEnv* env, jobject paint, jfloat scaleX) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setTextScaleX(SkFloatToScalar(scaleX));
    }

    static jfloat getTextSkewX(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        return SkScalarToFloat(GraphicsJNI::getNativePaint(env, paint)->getTextSkewX());
    }

    static void setTextSkewX(JNIEnv* env, jobject paint, jfloat skewX) {
        NPE_CHECK_RETURN_VOID(env, paint);
        GraphicsJNI::getNativePaint(env, paint)->setTextSkewX(SkFloatToScalar(skewX));
    }

    static jfloat ascent(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        SkPaint::FontMetrics    metrics;
        (void)GraphicsJNI::getNativePaint(env, paint)->getFontMetrics(&metrics);
        return SkScalarToFloat(metrics.fAscent);
    }

    static jfloat descent(JNIEnv* env, jobject paint) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        SkPaint::FontMetrics    metrics;
        (void)GraphicsJNI::getNativePaint(env, paint)->getFontMetrics(&metrics);
        return SkScalarToFloat(metrics.fDescent);
    }

    static jfloat getFontMetrics(JNIEnv* env, jobject paint, jobject metricsObj) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        SkPaint::FontMetrics metrics;
        SkScalar             spacing = GraphicsJNI::getNativePaint(env, paint)->getFontMetrics(&metrics);

        if (metricsObj) {
            SkASSERT(env->IsInstanceOf(metricsObj, gFontMetrics_class));
            env->SetFloatField(metricsObj, gFontMetrics_fieldID.top, SkScalarToFloat(metrics.fTop));
            env->SetFloatField(metricsObj, gFontMetrics_fieldID.ascent, SkScalarToFloat(metrics.fAscent));
            env->SetFloatField(metricsObj, gFontMetrics_fieldID.descent, SkScalarToFloat(metrics.fDescent));
            env->SetFloatField(metricsObj, gFontMetrics_fieldID.bottom, SkScalarToFloat(metrics.fBottom));
            env->SetFloatField(metricsObj, gFontMetrics_fieldID.leading, SkScalarToFloat(metrics.fLeading));
        }
        return SkScalarToFloat(spacing);
    }

    static jint getFontMetricsInt(JNIEnv* env, jobject paint, jobject metricsObj) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        SkPaint::FontMetrics metrics;

        GraphicsJNI::getNativePaint(env, paint)->getFontMetrics(&metrics);
        int ascent = SkScalarRound(metrics.fAscent);
        int descent = SkScalarRound(metrics.fDescent);
        int leading = SkScalarRound(metrics.fLeading);

        if (metricsObj) {
            SkASSERT(env->IsInstanceOf(metricsObj, gFontMetricsInt_class));
            env->SetIntField(metricsObj, gFontMetricsInt_fieldID.top, SkScalarFloor(metrics.fTop));
            env->SetIntField(metricsObj, gFontMetricsInt_fieldID.ascent, ascent);
            env->SetIntField(metricsObj, gFontMetricsInt_fieldID.descent, descent);
            env->SetIntField(metricsObj, gFontMetricsInt_fieldID.bottom, SkScalarCeil(metrics.fBottom));
            env->SetIntField(metricsObj, gFontMetricsInt_fieldID.leading, leading);
        }
        return descent - ascent + leading;
    }

    static jfloat measureText_CIII(JNIEnv* env, jobject jpaint, jcharArray text, jint index, jint count,
            jint bidiFlags) {
        NPE_CHECK_RETURN_ZERO(env, jpaint);
        NPE_CHECK_RETURN_ZERO(env, text);

        size_t textLength = env->GetArrayLength(text);
        if ((index | count) < 0 || (size_t)(index + count) > textLength) {
            doThrowAIOOBE(env);
            return 0;
        }
        if (count == 0) {
            return 0;
        }

        SkPaint* paint = GraphicsJNI::getNativePaint(env, jpaint);
        const jchar* textArray = env->GetCharArrayElements(text, NULL);
        jfloat result = 0;

        TextLayout::getTextRunAdvances(paint, textArray, index, count, textLength,
                bidiFlags, NULL /* dont need all advances */, &result);

        env->ReleaseCharArrayElements(text, const_cast<jchar*>(textArray), JNI_ABORT);
        return result;
    }

    static jfloat measureText_StringIII(JNIEnv* env, jobject jpaint, jstring text, jint start, jint end,
            jint bidiFlags) {
        NPE_CHECK_RETURN_ZERO(env, jpaint);
        NPE_CHECK_RETURN_ZERO(env, text);

        size_t textLength = env->GetStringLength(text);
        int count = end - start;
        if ((start | count) < 0 || (size_t)end > textLength) {
            doThrowAIOOBE(env);
            return 0;
        }
        if (count == 0) {
            return 0;
        }

        const jchar* textArray = env->GetStringChars(text, NULL);
        SkPaint* paint = GraphicsJNI::getNativePaint(env, jpaint);
        jfloat width = 0;

        TextLayout::getTextRunAdvances(paint, textArray, start, count, textLength,
                bidiFlags, NULL /* dont need all advances */, &width);

        env->ReleaseStringChars(text, textArray);
        return width;
    }

    static jfloat measureText_StringI(JNIEnv* env, jobject jpaint, jstring text, jint bidiFlags) {
        NPE_CHECK_RETURN_ZERO(env, jpaint);
        NPE_CHECK_RETURN_ZERO(env, text);

        size_t textLength = env->GetStringLength(text);
        if (textLength == 0) {
            return 0;
        }

        const jchar* textArray = env->GetStringChars(text, NULL);
        SkPaint* paint = GraphicsJNI::getNativePaint(env, jpaint);
        jfloat width = 0;

        TextLayout::getTextRunAdvances(paint, textArray, 0, textLength, textLength,
                bidiFlags, NULL /* dont need all advances */, &width);

        env->ReleaseStringChars(text, textArray);
        return width;
    }

    static int dotextwidths(JNIEnv* env, SkPaint* paint, const jchar text[], int count, jfloatArray widths,
            jint bidiFlags) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        NPE_CHECK_RETURN_ZERO(env, text);

        if (count < 0 || !widths) {
            doThrowAIOOBE(env);
            return 0;
        }
        if (count == 0) {
            return 0;
        }
        size_t widthsLength = env->GetArrayLength(widths);
        if ((size_t)count > widthsLength) {
            doThrowAIOOBE(env);
            return 0;
        }

        AutoJavaFloatArray autoWidths(env, widths, count);
        jfloat* widthsArray = autoWidths.ptr();

        TextLayout::getTextRunAdvances(paint, text, 0, count, count,
                bidiFlags, widthsArray, NULL /* dont need totalAdvance */);

        return count;
    }

    static jint getTextWidths___CIII_F(JNIEnv* env, jobject clazz, jlong paintHandle, jcharArray text,
            jint index, jint count, jint bidiFlags, jfloatArray widths) {
        SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        const jchar* textArray = env->GetCharArrayElements(text, NULL);
        count = dotextwidths(env, paint, textArray + index, count, widths, bidiFlags);
        env->ReleaseCharArrayElements(text, const_cast<jchar*>(textArray),
                                      JNI_ABORT);
        return count;
    }

    static jint getTextWidths__StringIII_F(JNIEnv* env, jobject clazz, jlong paintHandle, jstring text,
            jint start, jint end, jint bidiFlags, jfloatArray widths) {
        SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        const jchar* textArray = env->GetStringChars(text, NULL);
        int count = dotextwidths(env, paint, textArray + start, end - start, widths, bidiFlags);
        env->ReleaseStringChars(text, textArray);
        return count;
    }

    static int doTextGlyphs(JNIEnv* env, SkPaint* paint, const jchar* text, jint start, jint count,
            jint contextCount, jint flags, jcharArray glyphs) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        NPE_CHECK_RETURN_ZERO(env, text);

        if ((start | count | contextCount) < 0 || contextCount < count || !glyphs) {
            doThrowAIOOBE(env);
            return 0;
        }
        if (count == 0) {
            return 0;
        }
        size_t glypthsLength = env->GetArrayLength(glyphs);
        if ((size_t)count > glypthsLength) {
            doThrowAIOOBE(env);
            return 0;
        }

        jchar* glyphsArray = env->GetCharArrayElements(glyphs, NULL);

        sp<TextLayoutValue> value = TextLayoutEngine::getInstance().getValue(paint,
                text, start, count, contextCount, flags);
        const jchar* shapedGlyphs = value->getGlyphs();
        size_t glyphsCount = value->getGlyphsCount();
        memcpy(glyphsArray, shapedGlyphs, sizeof(jchar) * glyphsCount);

        env->ReleaseCharArrayElements(glyphs, glyphsArray, JNI_ABORT);
        return glyphsCount;
    }

    static jint getTextGlyphs__StringIIIII_C(JNIEnv* env, jobject clazz, jlong paintHandle,
            jstring text, jint start, jint end, jint contextStart, jint contextEnd, jint flags,
            jcharArray glyphs) {
        SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        const jchar* textArray = env->GetStringChars(text, NULL);
        int count = doTextGlyphs(env, paint, textArray + contextStart, start - contextStart,
                end - start, contextEnd - contextStart, flags, glyphs);
        env->ReleaseStringChars(text, textArray);
        return count;
    }

    static jfloat doTextRunAdvances(JNIEnv *env, SkPaint *paint, const jchar *text,
                                    jint start, jint count, jint contextCount, jint flags,
                                    jfloatArray advances, jint advancesIndex) {
        NPE_CHECK_RETURN_ZERO(env, paint);
        NPE_CHECK_RETURN_ZERO(env, text);

        if ((start | count | contextCount | advancesIndex) < 0 || contextCount < count) {
            doThrowAIOOBE(env);
            return 0;
        }
        if (count == 0) {
            return 0;
        }
        if (advances) {
            size_t advancesLength = env->GetArrayLength(advances);
            if ((size_t)count > advancesLength) {
                doThrowAIOOBE(env);
                return 0;
            }
        }
        jfloat advancesArray[count];
        jfloat totalAdvance = 0;

        TextLayout::getTextRunAdvances(paint, text, start, count, contextCount, flags,
                                       advancesArray, &totalAdvance);

        if (advances != NULL) {
            env->SetFloatArrayRegion(advances, advancesIndex, count, advancesArray);
        }
        return totalAdvance;
    }

    static jfloat getTextRunAdvances___CIIIII_FI(JNIEnv* env, jobject clazz, jlong paintHandle,
            jcharArray text, jint index, jint count, jint contextIndex, jint contextCount,
            jint flags, jfloatArray advances, jint advancesIndex) {
        SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        jchar* textArray = env->GetCharArrayElements(text, NULL);
        jfloat result = doTextRunAdvances(env, paint, textArray + contextIndex,
                index - contextIndex, count, contextCount, flags, advances, advancesIndex);
        env->ReleaseCharArrayElements(text, textArray, JNI_ABORT);
        return result;
    }

    static jfloat getTextRunAdvances__StringIIIII_FI(JNIEnv* env, jobject clazz, jlong paintHandle,
            jstring text, jint start, jint end, jint contextStart, jint contextEnd, jint flags,
            jfloatArray advances, jint advancesIndex) {
        SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        const jchar* textArray = env->GetStringChars(text, NULL);
        jfloat result = doTextRunAdvances(env, paint, textArray + contextStart,
                start - contextStart, end - start, contextEnd - contextStart, flags,
                advances, advancesIndex);
        env->ReleaseStringChars(text, textArray);
        return result;
    }

    static jint doTextRunCursor(JNIEnv *env, SkPaint* paint, const jchar *text, jint start,
            jint count, jint flags, jint offset, jint opt) {
        jfloat scalarArray[count];

        TextLayout::getTextRunAdvances(paint, text, start, count, start + count, flags,
                scalarArray, NULL /* dont need totalAdvance */);

        jint pos = offset - start;
        switch (opt) {
        case AFTER:
          if (pos < count) {
            pos += 1;
          }
          // fall through
        case AT_OR_AFTER:
          while (pos < count && scalarArray[pos] == 0) {
            ++pos;
          }
          break;
        case BEFORE:
          if (pos > 0) {
            --pos;
          }
          // fall through
        case AT_OR_BEFORE:
          while (pos > 0 && scalarArray[pos] == 0) {
            --pos;
          }
          break;
        case AT:
        default:
          if (scalarArray[pos] == 0) {
            pos = -1;
          }
          break;
        }

        if (pos != -1) {
          pos += start;
        }

        return pos;
    }

    static jint getTextRunCursor___C(JNIEnv* env, jobject clazz, jlong paintHandle, jcharArray text,
            jint contextStart, jint contextCount, jint flags, jint offset, jint cursorOpt) {
        SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        jchar* textArray = env->GetCharArrayElements(text, NULL);
        jint result = doTextRunCursor(env, paint, textArray, contextStart, contextCount, flags,
                offset, cursorOpt);
        env->ReleaseCharArrayElements(text, textArray, JNI_ABORT);
        return result;
    }

    static jint getTextRunCursor__String(JNIEnv* env, jobject clazz, jlong paintHandle, jstring text,
            jint contextStart, jint contextEnd, jint flags, jint offset, jint cursorOpt) {
        SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        const jchar* textArray = env->GetStringChars(text, NULL);
        jint result = doTextRunCursor(env, paint, textArray, contextStart,
                contextEnd - contextStart, flags, offset, cursorOpt);
        env->ReleaseStringChars(text, textArray);
        return result;
    }

    static void getTextPath(JNIEnv* env, SkPaint* paint, const jchar* text, jint count,
                            jint bidiFlags, jfloat x, jfloat y, SkPath *path) {
        TextLayout::getTextPath(paint, text, count, bidiFlags, x, y, path);
    }

    static void getTextPath___C(JNIEnv* env, jobject clazz, jlong paintHandle, jint bidiFlags,
            jcharArray text, jint index, jint count, jfloat x, jfloat y, jlong pathHandle) {
        SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        SkPath* path = reinterpret_cast<SkPath*>(pathHandle);
        const jchar* textArray = env->GetCharArrayElements(text, NULL);
        getTextPath(env, paint, textArray + index, count, bidiFlags, x, y, path);
        env->ReleaseCharArrayElements(text, const_cast<jchar*>(textArray), JNI_ABORT);
    }

    static void getTextPath__String(JNIEnv* env, jobject clazz, jlong paintHandle, jint bidiFlags,
            jstring text, jint start, jint end, jfloat x, jfloat y, jlong pathHandle) {
        SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        SkPath* path = reinterpret_cast<SkPath*>(pathHandle);
        const jchar* textArray = env->GetStringChars(text, NULL);
        getTextPath(env, paint, textArray + start, end - start, bidiFlags, x, y, path);
        env->ReleaseStringChars(text, textArray);
    }

    static void setShadowLayer(JNIEnv* env, jobject jpaint, jfloat radius,
                               jfloat dx, jfloat dy, jint color) {
        NPE_CHECK_RETURN_VOID(env, jpaint);

        SkPaint* paint = GraphicsJNI::getNativePaint(env, jpaint);
        if (radius <= 0) {
            paint->setLooper(NULL);
        }
        else {
            paint->setLooper(new SkBlurDrawLooper(SkFloatToScalar(radius),
                                                  SkFloatToScalar(dx),
                                                  SkFloatToScalar(dy),
                                                  (SkColor)color))->unref();
        }
    }

    static int breakText(JNIEnv* env, SkPaint& paint, const jchar text[],
                         int count, float maxWidth, jint bidiFlags, jfloatArray jmeasured,
                         SkPaint::TextBufferDirection tbd) {
        sp<TextLayoutValue> value = TextLayoutEngine::getInstance().getValue(&paint,
                text, 0, count, count, bidiFlags);
        if (value == NULL) {
            return 0;
        }
        SkScalar     measured;
        size_t       bytes = paint.breakText(value->getGlyphs(), value->getGlyphsCount() << 1,
                                   SkFloatToScalar(maxWidth), &measured, tbd);
        SkASSERT((bytes & 1) == 0);

        if (jmeasured && env->GetArrayLength(jmeasured) > 0) {
            AutoJavaFloatArray autoMeasured(env, jmeasured, 1);
            jfloat* array = autoMeasured.ptr();
            array[0] = SkScalarToFloat(measured);
        }
        return bytes >> 1;
    }

    static jint breakTextC(JNIEnv* env, jobject jpaint, jcharArray jtext,
            jint index, jint count, jfloat maxWidth, jint bidiFlags, jfloatArray jmeasuredWidth) {
        NPE_CHECK_RETURN_ZERO(env, jpaint);
        NPE_CHECK_RETURN_ZERO(env, jtext);

        SkPaint::TextBufferDirection tbd;
        if (count < 0) {
            tbd = SkPaint::kBackward_TextBufferDirection;
            count = -count;
        }
        else {
            tbd = SkPaint::kForward_TextBufferDirection;
        }

        if ((index < 0) || (index + count > env->GetArrayLength(jtext))) {
            doThrowAIOOBE(env);
            return 0;
        }

        SkPaint*     paint = GraphicsJNI::getNativePaint(env, jpaint);
        const jchar* text = env->GetCharArrayElements(jtext, NULL);
        count = breakText(env, *paint, text + index, count, maxWidth,
                          bidiFlags, jmeasuredWidth, tbd);
        env->ReleaseCharArrayElements(jtext, const_cast<jchar*>(text),
                                      JNI_ABORT);
        return count;
    }

    static jint breakTextS(JNIEnv* env, jobject jpaint, jstring jtext,
                jboolean forwards, jfloat maxWidth, jint bidiFlags, jfloatArray jmeasuredWidth) {
        NPE_CHECK_RETURN_ZERO(env, jpaint);
        NPE_CHECK_RETURN_ZERO(env, jtext);

        SkPaint::TextBufferDirection tbd = forwards ?
                                        SkPaint::kForward_TextBufferDirection :
                                        SkPaint::kBackward_TextBufferDirection;

        SkPaint* paint = GraphicsJNI::getNativePaint(env, jpaint);
        int count = env->GetStringLength(jtext);
        const jchar* text = env->GetStringChars(jtext, NULL);
        count = breakText(env, *paint, text, count, maxWidth, bidiFlags, jmeasuredWidth, tbd);
        env->ReleaseStringChars(jtext, text);
        return count;
    }

    static void doTextBounds(JNIEnv* env, const jchar* text, int count,
                             jobject bounds, const SkPaint& paint, jint bidiFlags) {
        SkRect  r;
        SkIRect ir;

        sp<TextLayoutValue> value = TextLayoutEngine::getInstance().getValue(&paint,
                text, 0, count, count, bidiFlags);
        if (value == NULL) {
            return;
        }
        paint.measureText(value->getGlyphs(), value->getGlyphsCount() << 1, &r);
        r.roundOut(&ir);
        GraphicsJNI::irect_to_jrect(ir, env, bounds);
    }

    static void getStringBounds(JNIEnv* env, jobject, jlong paintHandle,
                                jstring text, jint start, jint end, jint bidiFlags, jobject bounds) {
        const SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);;
        const jchar* textArray = env->GetStringChars(text, NULL);
        doTextBounds(env, textArray + start, end - start, bounds, *paint, bidiFlags);
        env->ReleaseStringChars(text, textArray);
    }

    static void getCharArrayBounds(JNIEnv* env, jobject, jlong paintHandle,
                        jcharArray text, jint index, jint count, jint bidiFlags, jobject bounds) {
        const SkPaint* paint = reinterpret_cast<SkPaint*>(paintHandle);
        const jchar* textArray = env->GetCharArrayElements(text, NULL);
        doTextBounds(env, textArray + index, count, bounds, *paint, bidiFlags);
        env->ReleaseCharArrayElements(text, const_cast<jchar*>(textArray),
                                      JNI_ABORT);
    }

};

static JNINativeMethod methods[] = {
    {"finalizer", "(J)V", (void*) SkPaintGlue::finalizer},
    {"native_init","()J", (void*) SkPaintGlue::init},
    {"native_initWithPaint","(J)J", (void*) SkPaintGlue::initWithPaint},
    {"native_reset","(J)V", (void*) SkPaintGlue::reset},
    {"native_set","(JJ)V", (void*) SkPaintGlue::assign},
    {"getFlags","()I", (void*) SkPaintGlue::getFlags},
    {"setFlags","(I)V", (void*) SkPaintGlue::setFlags},
    {"getHinting","()I", (void*) SkPaintGlue::getHinting},
    {"setHinting","(I)V", (void*) SkPaintGlue::setHinting},
    {"setAntiAlias","(Z)V", (void*) SkPaintGlue::setAntiAlias},
    {"setSubpixelText","(Z)V", (void*) SkPaintGlue::setSubpixelText},
    {"setLinearText","(Z)V", (void*) SkPaintGlue::setLinearText},
    {"setUnderlineText","(Z)V", (void*) SkPaintGlue::setUnderlineText},
    {"setStrikeThruText","(Z)V", (void*) SkPaintGlue::setStrikeThruText},
    {"setFakeBoldText","(Z)V", (void*) SkPaintGlue::setFakeBoldText},
    {"setFilterBitmap","(Z)V", (void*) SkPaintGlue::setFilterBitmap},
    {"setDither","(Z)V", (void*) SkPaintGlue::setDither},
    {"native_getStyle","(J)I", (void*) SkPaintGlue::getStyle},
    {"native_setStyle","(JI)V", (void*) SkPaintGlue::setStyle},
    {"getColor","()I", (void*) SkPaintGlue::getColor},
    {"setColor","(I)V", (void*) SkPaintGlue::setColor},
    {"getAlpha","()I", (void*) SkPaintGlue::getAlpha},
    {"setAlpha","(I)V", (void*) SkPaintGlue::setAlpha},
    {"getStrokeWidth","()F", (void*) SkPaintGlue::getStrokeWidth},
    {"setStrokeWidth","(F)V", (void*) SkPaintGlue::setStrokeWidth},
    {"getStrokeMiter","()F", (void*) SkPaintGlue::getStrokeMiter},
    {"setStrokeMiter","(F)V", (void*) SkPaintGlue::setStrokeMiter},
    {"native_getStrokeCap","(J)I", (void*) SkPaintGlue::getStrokeCap},
    {"native_setStrokeCap","(JI)V", (void*) SkPaintGlue::setStrokeCap},
    {"native_getStrokeJoin","(J)I", (void*) SkPaintGlue::getStrokeJoin},
    {"native_setStrokeJoin","(JI)V", (void*) SkPaintGlue::setStrokeJoin},
    {"native_getFillPath","(JJJ)Z", (void*) SkPaintGlue::getFillPath},
    {"native_setShader","(JJ)J", (void*) SkPaintGlue::setShader},
    {"native_setColorFilter","(JJ)J", (void*) SkPaintGlue::setColorFilter},
    {"native_setXfermode","(JJ)J", (void*) SkPaintGlue::setXfermode},
    {"native_setPathEffect","(JJ)J", (void*) SkPaintGlue::setPathEffect},
    {"native_setMaskFilter","(JJ)J", (void*) SkPaintGlue::setMaskFilter},
    {"native_setTypeface","(JJ)J", (void*) SkPaintGlue::setTypeface},
    {"native_setRasterizer","(JJ)J", (void*) SkPaintGlue::setRasterizer},
    {"native_getTextAlign","(J)I", (void*) SkPaintGlue::getTextAlign},
    {"native_setTextAlign","(JI)V", (void*) SkPaintGlue::setTextAlign},
    {"native_setTextLocale","(JLjava/lang/String;)V", (void*) SkPaintGlue::setTextLocale},
    {"getTextSize","()F", (void*) SkPaintGlue::getTextSize},
    {"setTextSize","(F)V", (void*) SkPaintGlue::setTextSize},
    {"getTextScaleX","()F", (void*) SkPaintGlue::getTextScaleX},
    {"setTextScaleX","(F)V", (void*) SkPaintGlue::setTextScaleX},
    {"getTextSkewX","()F", (void*) SkPaintGlue::getTextSkewX},
    {"setTextSkewX","(F)V", (void*) SkPaintGlue::setTextSkewX},
    {"ascent","()F", (void*) SkPaintGlue::ascent},
    {"descent","()F", (void*) SkPaintGlue::descent},
    {"getFontMetrics", "(Landroid/graphics/Paint$FontMetrics;)F", (void*)SkPaintGlue::getFontMetrics},
    {"getFontMetricsInt", "(Landroid/graphics/Paint$FontMetricsInt;)I", (void*)SkPaintGlue::getFontMetricsInt},
    {"native_measureText","([CIII)F", (void*) SkPaintGlue::measureText_CIII},
    {"native_measureText","(Ljava/lang/String;I)F", (void*) SkPaintGlue::measureText_StringI},
    {"native_measureText","(Ljava/lang/String;III)F", (void*) SkPaintGlue::measureText_StringIII},
    {"native_breakText","([CIIFI[F)I", (void*) SkPaintGlue::breakTextC},
    {"native_breakText","(Ljava/lang/String;ZFI[F)I", (void*) SkPaintGlue::breakTextS},
    {"native_getTextWidths","(J[CIII[F)I", (void*) SkPaintGlue::getTextWidths___CIII_F},
    {"native_getTextWidths","(JLjava/lang/String;III[F)I", (void*) SkPaintGlue::getTextWidths__StringIII_F},
    {"native_getTextRunAdvances","(J[CIIIII[FI)F",
        (void*) SkPaintGlue::getTextRunAdvances___CIIIII_FI},
    {"native_getTextRunAdvances","(JLjava/lang/String;IIIII[FI)F",
        (void*) SkPaintGlue::getTextRunAdvances__StringIIIII_FI},


    {"native_getTextGlyphs","(JLjava/lang/String;IIIII[C)I",
        (void*) SkPaintGlue::getTextGlyphs__StringIIIII_C},
    {"native_getTextRunCursor", "(J[CIIIII)I", (void*) SkPaintGlue::getTextRunCursor___C},
    {"native_getTextRunCursor", "(JLjava/lang/String;IIIII)I",
        (void*) SkPaintGlue::getTextRunCursor__String},
    {"native_getTextPath","(JI[CIIFFJ)V", (void*) SkPaintGlue::getTextPath___C},
    {"native_getTextPath","(JILjava/lang/String;IIFFJ)V", (void*) SkPaintGlue::getTextPath__String},
    {"nativeGetStringBounds", "(JLjava/lang/String;IIILandroid/graphics/Rect;)V",
                                        (void*) SkPaintGlue::getStringBounds },
    {"nativeGetCharArrayBounds", "(J[CIIILandroid/graphics/Rect;)V",
                                    (void*) SkPaintGlue::getCharArrayBounds },
    {"nSetShadowLayer", "(FFFI)V", (void*)SkPaintGlue::setShadowLayer}
};

static jfieldID req_fieldID(jfieldID id) {
    SkASSERT(id);
    return id;
}

int register_android_graphics_Paint(JNIEnv* env) {
    gFontMetrics_class = env->FindClass("android/graphics/Paint$FontMetrics");
    SkASSERT(gFontMetrics_class);
    gFontMetrics_class = (jclass)env->NewGlobalRef(gFontMetrics_class);

    gFontMetrics_fieldID.top = req_fieldID(env->GetFieldID(gFontMetrics_class, "top", "F"));
    gFontMetrics_fieldID.ascent = req_fieldID(env->GetFieldID(gFontMetrics_class, "ascent", "F"));
    gFontMetrics_fieldID.descent = req_fieldID(env->GetFieldID(gFontMetrics_class, "descent", "F"));
    gFontMetrics_fieldID.bottom = req_fieldID(env->GetFieldID(gFontMetrics_class, "bottom", "F"));
    gFontMetrics_fieldID.leading = req_fieldID(env->GetFieldID(gFontMetrics_class, "leading", "F"));

    gFontMetricsInt_class = env->FindClass("android/graphics/Paint$FontMetricsInt");
    SkASSERT(gFontMetricsInt_class);
    gFontMetricsInt_class = (jclass)env->NewGlobalRef(gFontMetricsInt_class);

    gFontMetricsInt_fieldID.top = req_fieldID(env->GetFieldID(gFontMetricsInt_class, "top", "I"));
    gFontMetricsInt_fieldID.ascent = req_fieldID(env->GetFieldID(gFontMetricsInt_class, "ascent", "I"));
    gFontMetricsInt_fieldID.descent = req_fieldID(env->GetFieldID(gFontMetricsInt_class, "descent", "I"));
    gFontMetricsInt_fieldID.bottom = req_fieldID(env->GetFieldID(gFontMetricsInt_class, "bottom", "I"));
    gFontMetricsInt_fieldID.leading = req_fieldID(env->GetFieldID(gFontMetricsInt_class, "leading", "I"));

    int result = AndroidRuntime::registerNativeMethods(env, "android/graphics/Paint", methods,
        sizeof(methods) / sizeof(methods[0]));
    return result;
}

}
