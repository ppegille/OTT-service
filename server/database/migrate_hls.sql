-- Migration script to add HLS support to existing database
-- Run this if you already have a database with videos
-- Date: 2025-11-12

-- Add HLS columns to videos table
ALTER TABLE videos ADD COLUMN hls_path TEXT;
ALTER TABLE videos ADD COLUMN hls_status TEXT DEFAULT 'pending';

-- Update existing videos to have pending HLS status
UPDATE videos SET hls_status = 'pending' WHERE hls_status IS NULL;

-- Verify migration
SELECT COUNT(*) as total_videos,
       SUM(CASE WHEN hls_status = 'pending' THEN 1 ELSE 0 END) as pending_hls,
       SUM(CASE WHEN hls_status = 'ready' THEN 1 ELSE 0 END) as ready_hls
FROM videos;
