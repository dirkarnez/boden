#pragma once

#include <bdn/View.h>
#include <bdn/ViewCoreFactory.h>
#include <bdn/ViewUtilities.h>

namespace bdn
{
    namespace detail
    {
        VIEW_CORE_REGISTRY_DECLARATION(Window)
    }

    class Window : public View
    {
      public:
      public:
        enum Orientation : int
        {
            Portrait = 0x1,
            LandscapeLeft = 0x2,
            LandscapeRight = 0x4,
            PortraitUpsideDown = 0x8,
            LandscapeMask = (LandscapeLeft | LandscapeRight),
            PortraitMask = (Portrait | PortraitUpsideDown),
            All = (LandscapeMask | PortraitMask)
        };

        static String orientationToString(Orientation orientation);

      public:
        Window(std::shared_ptr<ViewCoreFactory> viewCoreFactory = nullptr);

      private:
        Property<Rect> internalContentGeometry;

      public:
        const Property<Rect> &contentGeometry = internalContentGeometry;
        Property<String> title;
        Property<std::shared_ptr<View>> contentView;

        Property<Orientation> allowedOrientations;
        Property<Orientation> currentOrientation;

      public:
        std::list<std::shared_ptr<View>> childViews() override;
        void removeAllChildViews() override;

        void childViewStolen(const std::shared_ptr<View> &childView) override;

      protected:
        void bindViewCore() override;

      private:
        SingleChildHelper _contentView;

      public:
        class Core
        {
          public:
            using Orientation = Window::Orientation;

          public:
            Property<std::shared_ptr<View>> contentView;
            Property<Rect> contentGeometry;
            Property<String> title;

            Property<Orientation> allowedOrientations;
            Property<Orientation> currentOrientation;
        };
    };
}
